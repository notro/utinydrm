
// utinydrm version

#include <drm/drmP.h>
#include <drm/tinydrm/tinydrm.h>

// PATH_MAX
#include <linux/limits.h>

#include <errno.h>

// printf
#include <stdio.h>

// file
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// close
#include <unistd.h>

#include <sys/ioctl.h>


void tinydrm_lastclose(struct drm_device *drm)
{
	DRM_DEBUG_KMS("\n");
	drm_crtc_force_disable_all(drm);
}

void tinydrm_gem_cma_free_object(struct drm_gem_object *gem_obj)
{
}

struct drm_gem_object *
tinydrm_gem_cma_prime_import_sg_table(struct drm_device *drm,
				      struct dma_buf_attachment *attach,
				      struct sg_table *sgt)
{
	return NULL;
}

const struct file_operations tinydrm_fops = {
};

int devm_tinydrm_init(struct device *parent, struct tinydrm_device *tdev,
		      const struct drm_framebuffer_funcs *fb_funcs,
		      struct drm_driver *driver)
{
	struct drm_device *drm = &tdev->drm;
	struct utinydrm *udev = &drm->udev;

	tdev->drm.dev = parent;
	tdev->drm.driver = driver;
	tdev->drm.primary = &udev->minor;
	udev->fb_funcs = fb_funcs;
	tdev->pipe.plane.state = &tdev->pipe.plane.state_instance;

	return 0;
}

void tinydrm_unregister(struct tinydrm_device *tdev)
{
	struct drm_device *drm = &tdev->drm;
	struct utinydrm *udev = &drm->udev;

	DRM_DEBUG_KMS("\n");

	drm_crtc_force_disable_all(drm);

	close(udev->control_fd);
	close(udev->fd);
}

static const uint32_t udrm_formats[] = {
	DRM_FORMAT_RGB565,
};

int devm_tinydrm_register(struct tinydrm_device *tdev)
{
	struct drm_device *drm = &tdev->drm;
	struct utinydrm *udev = &drm->udev;
	struct udrm_dev_create udev_create;
	char ctrl_fname[PATH_MAX];
	int ret;

	memset(&udev_create, 0, sizeof(udev_create));
	udev_create.mode = udev->mode;
	udev_create.formats = (unsigned long)&udrm_formats;
	udev_create.num_formats = 1;
	udev_create.buf_mode = UDRM_BUF_MODE_SWAP_BYTES;
	strncpy(udev_create.name, drm->driver->name, UDRM_MAX_NAME_SIZE);

	udev->fd = open("/dev/udrm", O_RDWR);
	if (udev->fd == -1) {
		perror("Failed to open /dev/udrm");
		return -errno;
	}

	ret = ioctl(udev->fd, UDRM_DEV_CREATE, &udev_create);
	if (ret == -1) {
		perror("Failed to create device");
		close(udev->fd);
		return -errno;
	}

	snprintf(ctrl_fname, sizeof(ctrl_fname), "/dev/dri/controlD%d", udev_create.index + 64);
	DRM_DEBUG("DRM index: %d, ctrl_fname=%s\n", udev_create.index, ctrl_fname);

	udev->control_fd = open(ctrl_fname, O_RDWR);
	if (udev->control_fd == -1) {
		perror("Failed to open /dev/dri/...");
		close(udev->fd);
		return -errno;
	}

	drm->dev->utinydrm = udev;

	udev->buf_fd = udev_create.buf_fd;
	DRM_DEBUG_KMS("buf_fd=%d\n", udev_create.buf_fd);

	return 0;
}

void tinydrm_shutdown(struct tinydrm_device *tdev)
{
	struct drm_device *drm = &tdev->drm;

	drm_crtc_force_disable_all(drm);
}

int tinydrm_suspend(struct tinydrm_device *tdev)
{
	return 0;
}

int tinydrm_resume(struct tinydrm_device *tdev)
{
	return 0;
}

/* tinydrm-fb.c */

bool tinydrm_check_dirty(struct drm_framebuffer *fb,
			 struct drm_clip_rect **clips, unsigned int *num_clips)
{
	struct tinydrm_device *tdev = drm_to_tinydrm(fb->dev);

	if (!tdev->prepared)
		return false;

	/* Make sure to flush everything the first time */
	if (!tdev->enabled) {
		*clips = NULL;
		*num_clips = 0;
	}

	return true;
}

/* tinydrm-pipe.c */

void tinydrm_display_pipe_update(struct drm_simple_display_pipe *pipe,
				 struct drm_plane_state *old_state)
{
}

static int tinydrm_rotate_mode(struct drm_display_mode *mode,
			       unsigned int rotation)
{
	if (rotation == 0 || rotation == 180) {
		return 0;
	} else if (rotation == 90 || rotation == 270) {
		swap(mode->hdisplay, mode->vdisplay);
		swap(mode->hsync_start, mode->vsync_start);
		swap(mode->hsync_end, mode->vsync_end);
		swap(mode->htotal, mode->vtotal);
		swap(mode->width_mm, mode->height_mm);
		return 0;
	} else {
		return -EINVAL;
	}
}

int
tinydrm_display_pipe_init(struct tinydrm_device *tdev,
			  const struct drm_simple_display_pipe_funcs *funcs,
			  int connector_type,
			  const uint32_t *formats,
			  unsigned int format_count,
			  const struct drm_display_mode *mode,
			  unsigned int rotation)
{
	struct drm_device *drm = &tdev->drm;
	struct utinydrm *udev = &drm->udev;
	struct drm_display_mode mode_copy_stack;
	struct drm_display_mode *mode_copy = &mode_copy_stack;
	int ret;

	*mode_copy = *mode;
	ret = tinydrm_rotate_mode(mode_copy, rotation);
	if (ret) {
		DRM_ERROR("Illegal rotation value %u\n", rotation);
		return -EINVAL;
	}

	drm_mode_convert_to_umode(&udev->mode, mode_copy);

	//drm->mode_config.min_width = mode_copy->hdisplay;
	//drm->mode_config.max_width = mode_copy->hdisplay;
	//drm->mode_config.min_height = mode_copy->vdisplay;
	//drm->mode_config.max_height = mode_copy->vdisplay;

	tdev->pipe.funcs = funcs;
	//formats
	//format_count

	return 0;
}
