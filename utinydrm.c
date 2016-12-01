
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
#include <drm/tinydrm/mipi-dbi.h>
#include <linux/gpio/consumer.h>
#include <linux/spi/spi.h>

int drm_debug = 0xff;

static inline struct drm_device *
utinydrm_to_drm(struct utinydrm *udev)
{
	return container_of(udev, struct drm_device, udev);
}

static int utinydrm_pipe_enable(struct utinydrm *udev, struct utinydrm_event *ev)
{
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct tinydrm_device *tdev = drm_to_tinydrm(drm);

	if (tdev->pipe.funcs && tdev->pipe.funcs->enable)
		tdev->pipe.funcs->enable(&tdev->pipe, NULL);

	return 0;
}

static int utinydrm_pipe_disable(struct utinydrm *udev, struct utinydrm_event *ev)
{
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct tinydrm_device *tdev = drm_to_tinydrm(drm);

	if (tdev->pipe.funcs && tdev->pipe.funcs->disable)
		tdev->pipe.funcs->disable(&tdev->pipe);

	return 0;
}

static int utinydrm_fb_create(struct utinydrm *udev, struct utinydrm_event_fb_create *ev)
{
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct drm_mode_fb_cmd2 *mfb = &ev->fb;

	struct drm_mode_fb_cmd info = {
		.fb_id = mfb->fb_id,
	};
	int ret;
	struct drm_prime_handle prime;
	struct utinydrm_fb *ufb;
	struct drm_framebuffer *fb;

	DRM_DEBUG("[FB:%u] create: %ux%u, handles[0]=%u\n", mfb->fb_id, mfb->width, mfb->height, mfb->handles[0]);

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

	DRM_DEBUG("[FB:%u] create: %ux%u, handle=%u\n", info.fb_id, info.width, info.height, info.handle);
	ufb->handle = info.handle;

	memset(&prime, 0, sizeof(struct drm_prime_handle));
	prime.handle = info.handle;
	ret = ioctl(udev->control_fd, DRM_IOCTL_UTINYDRM_PRIME_HANDLE_TO_FD, &prime);
	if (ret == -1) {
		perror("Failed to get FD from prime handle");
		ret = -errno;
		goto error;
	}
	DRM_DEBUG("Prime Handle: %x to FD: %d\n", prime.handle, prime.fd);
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
	//printf("[FB:%u]: ufb=%p, ufb->next=%p\n", ufb->id, ufb, ufb->next);

	DRM_DEBUG("Address: %p\n", ufb->map);
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
	struct utinydrm_fb **prev, **curr, *ufb = NULL;
	int ret;

	DRM_DEBUG("[FB:%u] destroy\n", ev->fb_id);

	for (prev = &udev->fbs, curr = prev; *curr != NULL; prev = curr, curr = &(*curr)->next) {
		//printf("(*curr)->id=%u, *curr=%p, *curr->next=%p\n", (*curr)->id, *curr, (*curr)->next);
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

static int utinydrm_fb_dirty(struct utinydrm *udev, struct utinydrm_event_fb_dirty *ev)
{
	struct drm_mode_fb_dirty_cmd *dirty = &ev->fb_dirty_cmd;
	struct drm_device *drm = utinydrm_to_drm(udev);
	struct tinydrm_device *tdev = drm_to_tinydrm(drm);
	struct dma_buf_sync sync_args;
	struct drm_framebuffer *fb;
	struct utinydrm_fb *ufb;
	int ret;

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

	fb = &ufb->fb_cma.fb;
	tdev->pipe.plane.state->fb = fb;
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

int main(int argc, char const *argv[])
{
	struct tinydrm_device *tdev;
	struct utinydrm *udev;
	struct utinydrm_event *ev;
	int ret;
	struct pollfd pfd;
	struct spi_device spi_stack = {
		.dev = {
			.init_name = "spi0.0",
		},
		.master_instance = {
			.max_dma_len = (1 << 15), /* 32k */
			.bits_per_word_mask = SPI_BPW_MASK(8) | SPI_BPW_MASK(16),
		},
		.max_speed_hz = 32000000,
	};
	struct spi_device *spi = &spi_stack;

	ev = malloc(1024);
	if (!ev) {
		perror("Failed to allocate memory");
		return 1;
	}

	spi_add_device(spi);

	ret = driver->probe(spi);
	if (ret) {
		DRM_ERROR("driver probe failed %d\n", ret);
		return 1;
	}

	tdev = spi_get_drvdata(spi);
	udev = &tdev->drm.udev;

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
