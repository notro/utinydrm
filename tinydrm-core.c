
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
	udev->fb_funcs = fb_funcs;

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

int devm_tinydrm_register(struct tinydrm_device *tdev)
{
	struct drm_device *drm = &tdev->drm;
	struct utinydrm *udev = &drm->udev;
	struct utinydrm_dev_create udev_create;
	char ctrl_fname[PATH_MAX];
	int ret;

	memset(&udev_create, 0, sizeof(udev_create));
	udev_create.mode = udev->mode;
	strncpy(udev_create.name, drm->driver->name, UTINYDRM_MAX_NAME_SIZE);

	udev->fd = open("/dev/utinydrm", O_RDWR);
	if (udev->fd == -1) {
		perror("Failed to open /dev/utinydrm");
		return -errno;
	}

	ret = ioctl(udev->fd, UTINYDRM_DEV_CREATE, &udev_create);
	if (ret == -1) {
		perror("Failed to create device");
		close(udev->fd);
		return -errno;
	}

	snprintf(ctrl_fname, sizeof(ctrl_fname), "/dev/dri/controlD%d", udev_create.index + 64);
	printf("DRM index: %d, ctrl_fname=%s\n", udev_create.index, ctrl_fname);

	udev->control_fd = open(ctrl_fname, O_RDWR);
	if (udev->control_fd == -1) {
		perror("Failed to open /dev/dri/...");
		close(udev->fd);
		return -errno;
	}

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

	/* fbdev can flush even when we're not interested */
	if (tdev->pipe.plane.fb != fb)
		return false;

	/* Make sure to flush everything the first time */
	if (!tdev->enabled) {
		*clips = NULL;
		*num_clips = 0;
	}

	return true;
}
