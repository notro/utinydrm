/*
 * Copyright (C) 2016 Noralf Trønnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_TINYDRM_H
#define __LINUX_TINYDRM_H

#include <drm/drmP.h>
//#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_simple_kms_helper.h>

#include "../../../../tinydrm/include/uapi/drm/utinydrm.h"

struct tinydrm_debugfs_dirty;

struct tinydrm_device {
	struct drm_device drm;
	struct drm_simple_display_pipe pipe;
	struct work_struct dirty_work;
	struct mutex dev_lock;
	bool prepared;
	bool enabled;
//	struct drm_fbdev_cma *fbdev_cma;
//	struct drm_fb_helper *fbdev_helper;
//	bool fbdev_used;
//	struct drm_atomic_state *suspend_state;
//	struct tinydrm_debugfs_dirty *debugfs_dirty;
/* private: */
	const struct drm_framebuffer_funcs *fb_funcs;
};

static inline struct tinydrm_device *
drm_to_tinydrm(struct drm_device *drm)
{
	return container_of(drm, struct tinydrm_device, drm);
}

static inline struct tinydrm_device *
pipe_to_tinydrm(struct drm_simple_display_pipe *pipe)
{
	return container_of(pipe, struct tinydrm_device, pipe);
}

#define TINYDRM_GEM_DRIVER_OPS \
	.fops			= &tinydrm_fops

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

extern const struct file_operations tinydrm_fops;
void tinydrm_lastclose(struct drm_device *drm);
void tinydrm_gem_cma_free_object(struct drm_gem_object *gem_obj);
struct drm_gem_object *
tinydrm_gem_cma_prime_import_sg_table(struct drm_device *drm,
				      struct dma_buf_attachment *attach,
				      struct sg_table *sgt);
struct drm_framebuffer *
tinydrm_fb_create(struct drm_device *drm, struct drm_file *file_priv,
		  const struct drm_mode_fb_cmd2 *mode_cmd);
bool tinydrm_check_dirty(struct drm_framebuffer *fb,
			 struct drm_clip_rect **clips,
			 unsigned int *num_clips);
struct drm_connector *
tinydrm_connector_create(struct drm_device *drm,
			 const struct drm_display_mode *mode,
			 int connector_type);
void tinydrm_display_pipe_update(struct drm_simple_display_pipe *pipe,
				 struct drm_plane_state *old_state);
int
tinydrm_display_pipe_init(struct tinydrm_device *tdev,
			  const struct drm_simple_display_pipe_funcs *funcs,
			  int connector_type,
			  const uint32_t *formats,
			  unsigned int format_count,
			  const struct drm_display_mode *mode,
			  unsigned int rotation);
int devm_tinydrm_init(struct device *parent, struct tinydrm_device *tdev,
		      const struct drm_framebuffer_funcs *fb_funcs,
		      struct drm_driver *driver);
int devm_tinydrm_register(struct tinydrm_device *tdev);
void tinydrm_unregister(struct tinydrm_device *tdev);
void tinydrm_shutdown(struct tinydrm_device *tdev);
int tinydrm_suspend(struct tinydrm_device *tdev);
int tinydrm_resume(struct tinydrm_device *tdev);

int tinydrm_fbdev_init(struct tinydrm_device *tdev);
void tinydrm_fbdev_fini(struct tinydrm_device *tdev);

static inline int tinydrm_debugfs_dirty_init(struct tinydrm_device *tdev)
{
	return 0;
}

static inline void tinydrm_debugfs_dirty_begin(struct tinydrm_device *tdev,
				 struct drm_framebuffer *fb,
				 const struct drm_clip_rect *clip)
{
}

static inline void tinydrm_debugfs_dirty_end(struct tinydrm_device *tdev, size_t len,
			       unsigned int bits_per_pixel)
{
}

#define tinydrm_debugfs_init	NULL
#define tinydrm_debugfs_cleanup	NULL

struct bo
{
	int fd;
	void *ptr;
	size_t size;
	size_t offset;
	size_t pitch;
	unsigned handle;
};

struct udmabuf {
	struct bo *bo;
	int fd;
	void *map;
	size_t map_size;
};

extern struct udmabuf *tx_buf;
void *utinydrm_get_tx_buf(struct device *dev, size_t len);

#endif /* __LINUX_TINYDRM_H */
