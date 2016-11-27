/*
 * uledmon.c
 *
 * This program creates a new userspace LED class device and monitors it. A
 * timestamp and brightness value is printed each time the brightness changes.
 *
 * Usage: uledmon <device-name>
 *
 * <device-name> is the name of the LED class device to be created. Pressing
 * CTRL+C will exit.
 */

#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include <linux/dma-buf.h>

//#include <drm/utinydrm.h>
#include "../tinydrm/include/uapi/drm/utinydrm.h"


struct utinydrm_fb {
	unsigned int id;
	unsigned int handle;
	int buf_fd;
	off_t map_size;
	void *map;
	struct utinydrm_fb *next;
};

struct utinydrm {
	int fd;
	int control_fd;
	struct utinydrm_fb *fbs;
};

struct drm_display_mode {
//        struct drm_mode_object base;
        char name[DRM_DISPLAY_MODE_LEN];
//        enum drm_mode_status status;
        unsigned int type;
        int clock;              /* in kHz */
        int hdisplay;
        int hsync_start;
        int hsync_end;
        int htotal;
        int hskew;
        int vdisplay;
        int vsync_start;
        int vsync_end;
        int vtotal;
        int vscan;
        unsigned int flags;
        int width_mm;
        int height_mm;
        int crtc_clock;
        int crtc_hdisplay;
        int crtc_hblank_start;
        int crtc_hblank_end;
        int crtc_hsync_start;
        int crtc_hsync_end;
        int crtc_htotal;
        int crtc_hskew;
        int crtc_vdisplay;
        int crtc_vblank_start;
        int crtc_vblank_end;
        int crtc_vsync_start;
        int crtc_vsync_end;
        int crtc_vtotal;
        int *private;
        int private_flags;
        int vrefresh;
        int hsync;
//        enum hdmi_picture_aspect picture_aspect_ratio;
};


static int utinydrm_pipe_enable(struct utinydrm *udev, struct utinydrm_event *ev)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: Pipe enable\n", ts.tv_sec, ts.tv_nsec, ev->type);

	return 0;
}

static int utinydrm_pipe_disable(struct utinydrm *udev, struct utinydrm_event *ev)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: Pipe disable\n", ts.tv_sec, ts.tv_nsec, ev->type);

	return 0;
}

static int utinydrm_fb_create(struct utinydrm *udev, struct utinydrm_event_fb_create *ev)
{
	struct drm_mode_fb_cmd2 *mfb = &ev->fb;
	struct timespec ts;

	struct drm_mode_fb_cmd info = {
		.fb_id = mfb->fb_id,
	};
	int ret;
	struct drm_prime_handle prime;
	struct utinydrm_fb *ufb;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: [FB:%u] create: %ux%u, handles[0]=%u\n", ts.tv_sec, ts.tv_nsec, ev->base.type, mfb->fb_id, mfb->width, mfb->height, mfb->handles[0]);

	ufb = calloc(1, sizeof(*ufb));
	if (!ufb)
		return -ENOMEM;

	ufb->id = info.fb_id;

	ret = ioctl(udev->control_fd, DRM_IOCTL_MODE_GETFB, &info);
	if (ret == -1) {
		perror("Failed to get fb");
		ret = -errno;
		goto error;
	}

	printf("[%ld.%09ld] [FB:%u] create: %ux%u, handle=%u\n", ts.tv_sec, ts.tv_nsec, info.fb_id, info.width, info.height, info.handle);
	ufb->handle = info.handle;

	memset(&prime, 0, sizeof(struct drm_prime_handle));
	prime.handle = info.handle;
	ret = ioctl(udev->control_fd, DRM_IOCTL_UTINYDRM_PRIME_HANDLE_TO_FD, &prime);
	if (ret == -1) {
		perror("Failed to get FD from prime handle");
		ret = -errno;
		goto error;
	}
	printf("Prime Handle: %x to FD: %d\n", prime.handle, prime.fd);
	ufb->buf_fd = prime.fd;

	ufb->map_size = info.height * info.pitch;

	//ufb->map = mmap(NULL, ufb->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, prime.fd, 0);
	ufb->map = mmap(NULL, ufb->map_size, PROT_READ, MAP_SHARED, prime.fd, 0);
	if (ufb->map == MAP_FAILED) {
		perror("Failed to mmap");
		ret = -errno;
		goto error;
	}

	ufb->next = udev->fbs;
	udev->fbs = ufb;
	printf("[FB:%u]: ufb=%p, ufb->next=%p\n", ufb->id, ufb, ufb->next);

	printf("Address: %p\n", ufb->map);

	return 0;

error:
	free(ufb);

	return ret;
}

static int utinydrm_fb_destroy(struct utinydrm *udev, struct utinydrm_event_fb_destroy *ev)
{
	struct timespec ts;
	struct utinydrm_fb **prev, **curr, *ufb = NULL;
	int ret;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: [FB:%u] destroy\n", ts.tv_sec, ts.tv_nsec, ev->base.type, ev->fb_id);

	for (prev = &udev->fbs, curr = prev; *curr != NULL; prev = curr, curr = &(*curr)->next) {
		printf("(*curr)->id=%u, *curr=%p, *curr->next=%p\n", (*curr)->id, *curr, (*curr)->next);
		if ((*curr)->id == ev->fb_id) {
			ufb = *curr;
			*prev = (*curr)->next;
			break;
		}
	}

	if (!ufb) {
		printf("%s: Failed to find framebuffer %u\n", __func__, ev->fb_id);
		return -EINVAL;
	}

	ret = munmap(ufb->map, ufb->map_size);
	if (ret == -1) {
		perror("Failed to munmap");
		ret = -errno;
		//goto error;
	}

	return 0;
}

// http://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
static void hexdump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

static int utinydrm_fb_dirty(struct utinydrm *udev, struct utinydrm_event_fb_dirty *ev)
{
	struct utinydrm_fb *ufb;
	struct timespec ts;
	struct drm_clip_rect *clip;
	struct drm_mode_fb_dirty_cmd *dirty = &ev->fb_dirty_cmd;
	struct dma_buf_sync sync_args;
	int i, ret;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: [FB:%u] dirty: num_clips=%u\n", ts.tv_sec, ts.tv_nsec, ev->base.type, dirty->fb_id, dirty->num_clips);

	for (i = 0; i < dirty->num_clips; i++) {
		clip = &ev->clips[i];

		printf("Flushing [FB:%d] x1=%u, x2=%u, y1=%u, y2=%u\n", dirty->fb_id,
			clip->x1, clip->x2, clip->y1, clip->y2);
	}

	for (ufb = udev->fbs; ufb != NULL; ufb = ufb->next) {
		if (ufb->id == ev->fb_dirty_cmd.fb_id)
			break;
	}

	if (!ufb)
		return 0;

	sync_args.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ;
	ret = ioctl(ufb->buf_fd, DMA_BUF_IOCTL_SYNC, &sync_args);
	if (ret == -1)
		perror("Failed to DMA_BUF_SYNC_START");

	hexdump("utinydrm_fb_dirty", ufb->map, 320 * 2);

	sync_args.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
	ret = ioctl(ufb->buf_fd, DMA_BUF_IOCTL_SYNC, &sync_args);
	if (ret == -1)
		perror("Failed to DMA_BUF_SYNC_END");

	return 0;
}

static int utinydrm_event(struct utinydrm *udev, struct utinydrm_event *ev)
{
	int ret;

	usleep(100000);

	switch (ev->type) {
	case UTINYDRM_EVENT_PIPE_ENABLE:
		ret = utinydrm_pipe_enable(udev, ev);
		break;
	case UTINYDRM_EVENT_PIPE_DISABLE:
		ret = utinydrm_pipe_disable(udev, ev);
		break;
	case UTINYDRM_EVENT_FB_CREATE:
		ret = utinydrm_fb_create(udev, (struct utinydrm_event_fb_create *)ev);
		break;
	case UTINYDRM_EVENT_FB_DESTROY:
		ret = utinydrm_fb_destroy(udev, (struct utinydrm_event_fb_destroy *)ev);
		break;
	case UTINYDRM_EVENT_FB_DIRTY:
		ret = utinydrm_fb_dirty(udev, (struct utinydrm_event_fb_dirty *)ev);
		break;
	default:
		printf("Unknown event: %u\n", ev->type);
		ret = 0;
		break;
	}

	usleep(100000);

	return ret;
}

static void drm_mode_convert_to_umode(struct drm_mode_modeinfo *out,
                                      const struct drm_display_mode *in)
{
        out->clock = in->clock;
        out->hdisplay = in->hdisplay;
        out->hsync_start = in->hsync_start;
        out->hsync_end = in->hsync_end;
        out->htotal = in->htotal;
        out->hskew = in->hskew;
        out->vdisplay = in->vdisplay;
        out->vsync_start = in->vsync_start;
        out->vsync_end = in->vsync_end;
        out->vtotal = in->vtotal;
        out->vscan = in->vscan;
        out->vrefresh = in->vrefresh;
        out->flags = in->flags;
        out->type = in->type;
        strncpy(out->name, in->name, DRM_DISPLAY_MODE_LEN);
        out->name[DRM_DISPLAY_MODE_LEN-1] = 0;
}

#define TINYDRM_MODE(hd, vd, hd_mm, vd_mm) \
	.hdisplay = (hd), \
	.hsync_start = (hd), \
	.hsync_end = (hd), \
	.htotal = (hd), \
	.vdisplay = (vd), \
	.vsync_start = (vd), \
	.vsync_end = (vd), \
	.vtotal = (vd), \
	.width_mm = (hd_mm), \
	.height_mm = (vd_mm), \
	.type = DRM_MODE_TYPE_DRIVER, \
	.clock = 1 /* pass validation */

static const struct drm_display_mode utinydrm_mode = {
	TINYDRM_MODE(320, 240, 58, 43),
};

int main(int argc, char const *argv[])
{
	struct utinydrm udev;
	struct utinydrm_dev_create udev_create = {
		.name = "mi0283qt",
	};
	struct utinydrm_event *ev;
	int ret;
	struct pollfd pfd;
	char ctrl_fname[PATH_MAX];

	memset(&udev, 0, sizeof(udev));

	drm_mode_convert_to_umode(&udev_create.mode, &utinydrm_mode);

	ev = malloc(1024);
	if (!ev) {
		perror("Failed to allocate memory");
		return 1;
	}

	udev.fd = open("/dev/utinydrm", O_RDWR);
	if (udev.fd == -1) {
		perror("Failed to open /dev/utinydrm");
		return 1;
	}

	ret = ioctl(udev.fd, UTINYDRM_DEV_CREATE, &udev_create);
	if (ret == -1) {
		perror("Failed to create device");
		close(udev.fd);
		return 1;
	}

	snprintf(ctrl_fname, sizeof(ctrl_fname), "/dev/dri/controlD%d", udev_create.index + 64);
	printf("DRM index: %d, ctrl_fname=%s\n", udev_create.index, ctrl_fname);

	udev.control_fd = open(ctrl_fname, O_RDWR);
	if (udev.control_fd == -1) {
		perror("Failed to open /dev/dri/...");
		close(udev.fd);
		return 1;
	}

	pfd.fd = udev.fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	while (!(pfd.revents & (POLLERR | POLLHUP | POLLNVAL))) {
		int event_ret;

		ret = read(udev.fd, ev, 1024);
		if (ret == -1) {
			perror("Failed to read from /dev/utinydrm");
			close(udev.control_fd);
			close(udev.fd);
			return 1;
		}

		event_ret = utinydrm_event(&udev, ev);

		ret = write(udev.fd, &event_ret, sizeof(int));
		if (ret == -1) {
			perror("Failed to write to /dev/utinydrm");
			close(udev.control_fd);
			close(udev.fd);
			return 1;
		}

		poll(&pfd, 1, -1);
	}

	close(udev.control_fd);
	close(udev.fd);

	return 0;
}
