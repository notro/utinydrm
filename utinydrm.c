
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

#include <drm/tinydrm/tinydrm.h>
#include <drm/tinydrm/tinydrm-helpers.h>

static inline struct drm_device *
utinydrm_to_drm(struct utinydrm *udev)
{
	return container_of(udev, struct drm_device, udev);
}

static int utinydrm_pipe_enable(struct utinydrm *udev, struct utinydrm_event *ev)
{
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct tinydrm_device *tdev = drm_to_tinydrm(drm);
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: Pipe enable\n", ts.tv_sec, ts.tv_nsec, ev->type);
	tdev->prepared = true;

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
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct drm_mode_fb_cmd2 *mfb = &ev->fb;
	struct timespec ts;

	struct drm_mode_fb_cmd info = {
		.fb_id = mfb->fb_id,
	};
	int ret;
	struct drm_prime_handle prime;
	struct utinydrm_fb *ufb;
	struct drm_framebuffer *fb;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%ld.%09ld] %u: [FB:%u] create: %ux%u, handles[0]=%u\n", ts.tv_sec, ts.tv_nsec, ev->base.type, mfb->fb_id, mfb->width, mfb->height, mfb->handles[0]);

	ufb = calloc(1, sizeof(*ufb));
	if (!ufb)
		return -ENOMEM;

	fb = &ufb->fb_cma.fb;
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
	ufb->fb_cma.obj[0] = &ufb->cma_obj;
	ufb->fb_cma.obj[0]->vaddr = ufb->map;

	fb->base.id = info.fb_id;
	fb->dev = drm;
	fb->funcs = udev->fb_funcs;
	fb->pitches[0] = info.pitch;
	fb->width = info.width;
	fb->height = info.height;
	//fb->bpp = info.bpp;
	//fb->depth = info.depth
	fb->pixel_format = drm_mode_legacy_fb_format(info.bpp, info.depth);

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
	struct drm_framebuffer *fb;
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

	fb = &ufb->fb_cma.fb;
	if (fb && fb->funcs && fb->funcs->dirty)
		fb->funcs->dirty(fb, NULL, dirty->flags, dirty->color, ev->clips, dirty->num_clips);

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

/*********************************************************************************************************************************/

static const struct drm_display_mode utinydrm_mode = {
	TINYDRM_MODE(320, 240, 58, 43),
};

static int u_fb_dirty(struct drm_framebuffer *fb,
			     struct drm_file *file_priv,
			     unsigned int flags, unsigned int color,
			     struct drm_clip_rect *clips,
			     unsigned int num_clips)
{
	struct drm_gem_cma_object *cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
	struct tinydrm_device *tdev = drm_to_tinydrm(fb->dev);
	struct drm_clip_rect clip;
	int ret = 0;

printf("%s(fb=%p, clips=%p, num_clips=%u)\n", __func__, fb, clips, num_clips);
	mutex_lock(&tdev->dev_lock);

	if (!tinydrm_check_dirty(fb, &clips, &num_clips))
		goto out_unlock;

	tinydrm_merge_clips(&clip, clips, num_clips, flags,
			    fb->width, fb->height);
	clip.x1 = 0;
	clip.x2 = fb->width;

	DRM_DEBUG("Flushing [FB:%d] x1=%u, x2=%u, y1=%u, y2=%u\n", fb->base.id,
		  clip.x1, clip.x2, clip.y1, clip.y2);

	printf("Address: %p\n", cma_obj->vaddr);

	tinydrm_debugfs_dirty_begin(tdev, fb, &clip);

	tinydrm_debugfs_dirty_end(tdev, 0, 16);

	if (ret) {
		dev_err_once(fb->dev->dev, "Failed to update display %d\n",
			     ret);
		goto out_unlock;
	}

out_unlock:
	mutex_unlock(&tdev->dev_lock);

	return ret;
}

static const struct drm_framebuffer_funcs u_fb_funcs = {
	.dirty = u_fb_dirty,
};

static struct drm_driver udrv = {
	.name = "mi0283qt",
};

int main(int argc, char const *argv[])
{
	struct tinydrm_device tdev_stack;
	struct tinydrm_device *tdev = &tdev_stack;
	struct device dev;
	struct utinydrm *udev;
	struct utinydrm_event *ev;
	int ret;
	struct pollfd pfd;

	memset(&udev, 0, sizeof(udev));
	memset(tdev, 0, sizeof(*tdev));

	udev = &tdev->drm.udev;

	drm_mode_convert_to_umode(&udev->mode, &utinydrm_mode);

	ev = malloc(1024);
	if (!ev) {
		perror("Failed to allocate memory");
		return 1;
	}

	devm_tinydrm_init(&dev, tdev, &u_fb_funcs, &udrv);
	ret = devm_tinydrm_register(tdev);
	if (ret)
		return 1;

	pfd.fd = udev->fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	while (!(pfd.revents & (POLLERR | POLLHUP | POLLNVAL))) {
		int event_ret;

		ret = read(udev->fd, ev, 1024);
		if (ret == -1) {
			perror("Failed to read from /dev/utinydrm");
			tinydrm_unregister(tdev);
			return 1;
		}

		event_ret = utinydrm_event(udev, ev);

		ret = write(udev->fd, &event_ret, sizeof(int));
		if (ret == -1) {
			perror("Failed to write to /dev/utinydrm");
			tinydrm_unregister(tdev);
			return 1;
		}

		poll(&pfd, 1, -1);
	}

	tinydrm_unregister(tdev);

	return 0;
}
