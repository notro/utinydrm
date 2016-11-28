#ifndef _DRM_P_H_
#define _DRM_P_H_

#include <stdint.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <drm/drm_mode.h>
#include <drm/drm_fourcc.h>

// off_t
#include <sys/types.h>

struct drm_framebuffer;
struct drm_file;

struct drm_framebuffer_funcs {
	void (*destroy)(struct drm_framebuffer *framebuffer);

	int (*create_handle)(struct drm_framebuffer *fb,
			     struct drm_file *file_priv,
			     unsigned int *handle);

	int (*dirty)(struct drm_framebuffer *framebuffer,
		     struct drm_file *file_priv, unsigned flags,
		     unsigned color, struct drm_clip_rect *clips,
		     unsigned num_clips);
};

struct drm_framebuffer {
	struct drm_device *dev;
//	struct list_head head;
//	struct drm_mode_object base;
	const struct drm_framebuffer_funcs *funcs;
	unsigned int pitches[4];
	unsigned int offsets[4];
	uint64_t modifier[4];
	unsigned int width;
	unsigned int height;
//	unsigned int depth;
//	int bits_per_pixel;
//	int flags;
	uint32_t pixel_format; /* fourcc format */
//	int hot_x;
//	int hot_y;
//	struct list_head filp_head;
};

struct utinydrm_fb {
	struct drm_framebuffer fb;
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
	struct drm_mode_modeinfo mode;
	const struct drm_framebuffer_funcs *fb_funcs;
	struct utinydrm_fb *fbs;
};



struct drm_minor {

};

struct drm_file {

};

struct drm_plane_state {

};

struct drm_crtc_state {

};

struct drm_crtc {

};

struct drm_plane {

};

struct drm_encoder {

};

struct drm_bridge {

};

struct drm_gem_object {

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





struct dma_buf_attachment {

};





struct drm_device {
	struct utinydrm udev;
#if 0
	struct list_head legacy_dev_list;/**< list of devices per driver for stealth attach cleanup */
	int if_version;			/**< Highest interface version set */

	/** \name Lifetime Management */
	/*@{ */
	struct kref ref;		/**< Object ref-count */
#endif
	struct device *dev;		/**< Device structure of bus-device */
	struct drm_driver *driver;	/**< DRM driver managing the device */
#if 0
	void *dev_private;		/**< DRM driver private data */
	struct drm_minor *control;		/**< Control node */
	struct drm_minor *primary;		/**< Primary node */
	struct drm_minor *render;		/**< Render node */

	/* currently active master for this device. Protected by master_mutex */
	struct drm_master *master;

	atomic_t unplugged;			/**< Flag whether dev is dead */
	struct inode *anon_inode;		/**< inode for private address-space */
	char *unique;				/**< unique name of the device */
	/*@} */

	/** \name Locks */
	/*@{ */
	struct mutex struct_mutex;	/**< For others */
	struct mutex master_mutex;      /**< For drm_minor::master and drm_file::is_master */
	/*@} */

	/** \name Usage Counters */
	/*@{ */
	int open_count;			/**< Outstanding files open, protected by drm_global_mutex. */
	spinlock_t buf_lock;		/**< For drm_device::buf_use and a few other things. */
	int buf_use;			/**< Buffers in use -- cannot alloc */
	atomic_t buf_alloc;		/**< Buffer allocation in progress */
	/*@} */

	struct mutex filelist_mutex;
	struct list_head filelist;

	/** \name Memory management */
	/*@{ */
	struct list_head maplist;	/**< Linked list of regions */
	struct drm_open_hash map_hash;	/**< User token hash table for maps */

	/** \name Context handle management */
	/*@{ */
	struct list_head ctxlist;	/**< Linked list of context handles */
	struct mutex ctxlist_mutex;	/**< For ctxlist */

	struct idr ctx_idr;

	struct list_head vmalist;	/**< List of vmas (for debugging) */

	/*@} */

	/** \name DMA support */
	/*@{ */
	struct drm_device_dma *dma;		/**< Optional pointer for DMA support */
	/*@} */

	/** \name Context support */
	/*@{ */

	__volatile__ long context_flag;	/**< Context swapping flag */
	int last_context;		/**< Last current context */
	/*@} */

	/** \name VBLANK IRQ support */
	/*@{ */
	bool irq_enabled;
	int irq;

	/*
	 * If true, vblank interrupt will be disabled immediately when the
	 * refcount drops to zero, as opposed to via the vblank disable
	 * timer.
	 * This can be set to true it the hardware has a working vblank
	 * counter and the driver uses drm_vblank_on() and drm_vblank_off()
	 * appropriately.
	 */
	bool vblank_disable_immediate;

	/* array of size num_crtcs */
	struct drm_vblank_crtc *vblank;

	spinlock_t vblank_time_lock;    /**< Protects vblank count and time updates during vblank enable/disable */
	spinlock_t vbl_lock;

	u32 max_vblank_count;           /**< size of vblank counter register */

	/**
	 * List of events
	 */
	struct list_head vblank_event_list;
	spinlock_t event_lock;

	struct drm_sg_mem *sg;	/**< Scatter gather memory */
	unsigned int num_crtcs;                  /**< Number of CRTCs on this device */

	struct {
		int context;
		struct drm_hw_lock *lock;
	} sigdata;

	struct drm_local_map *agp_buffer_map;
	unsigned int agp_buffer_token;

	struct drm_mode_config mode_config;	/**< Current mode config */

	/** \name GEM information */
	/*@{ */
	struct mutex object_name_lock;
	struct idr object_name_idr;
	struct drm_vma_offset_manager *vma_offset_manager;
	/*@} */
	int switch_power_state;
#endif
};

struct drm_driver {
//	int (*load) (struct drm_device *, unsigned long flags);
//	int (*firstopen) (struct drm_device *);
//	int (*open) (struct drm_device *, struct drm_file *);
//	void (*preclose) (struct drm_device *, struct drm_file *file_priv);
//	void (*postclose) (struct drm_device *, struct drm_file *);
	void (*lastclose) (struct drm_device *);
//	int (*unload) (struct drm_device *);
//	int (*dma_ioctl) (struct drm_device *dev, void *data, struct drm_file *file_priv);
//	int (*dma_quiescent) (struct drm_device *);
//	int (*context_dtor) (struct drm_device *dev, int context);
//	int (*set_busid)(struct drm_device *dev, struct drm_master *master);

//	u32 (*get_vblank_counter) (struct drm_device *dev, unsigned int pipe);
//
//	int (*enable_vblank) (struct drm_device *dev, unsigned int pipe);
//
//	void (*disable_vblank) (struct drm_device *dev, unsigned int pipe);
//
//	int (*device_is_agp) (struct drm_device *dev);
//
//	int (*get_scanout_position) (struct drm_device *dev, unsigned int pipe,
//				     unsigned int flags, int *vpos, int *hpos,
//				     ktime_t *stime, ktime_t *etime,
//				     const struct drm_display_mode *mode);
//
//	int (*get_vblank_timestamp) (struct drm_device *dev, unsigned int pipe,
//				     int *max_error,
//				     struct timeval *vblank_time,
//				     unsigned flags);
//
//	irqreturn_t(*irq_handler) (int irq, void *arg);
//	void (*irq_preinstall) (struct drm_device *dev);
//	int (*irq_postinstall) (struct drm_device *dev);
//	void (*irq_uninstall) (struct drm_device *dev);

//	/* Master routines */
//	int (*master_create)(struct drm_device *dev, struct drm_master *master);
//	void (*master_destroy)(struct drm_device *dev, struct drm_master *master);
//	/**
//	 * master_set is called whenever the minor master is set.
//	 * master_drop is called whenever the minor master is dropped.
//	 */
//
//	int (*master_set)(struct drm_device *dev, struct drm_file *file_priv,
//			  bool from_open);
//	void (*master_drop)(struct drm_device *dev, struct drm_file *file_priv);
//
	int (*debugfs_init)(struct drm_minor *minor);
	void (*debugfs_cleanup)(struct drm_minor *minor);

	void (*gem_free_object) (struct drm_gem_object *obj);
	void (*gem_free_object_unlocked) (struct drm_gem_object *obj);
	int (*gem_open_object) (struct drm_gem_object *, struct drm_file *);
	void (*gem_close_object) (struct drm_gem_object *, struct drm_file *);
	struct drm_gem_object *(*gem_create_object)(struct drm_device *dev,
						    size_t size);
	int (*prime_handle_to_fd)(struct drm_device *dev, struct drm_file *file_priv,
				uint32_t handle, uint32_t flags, int *prime_fd);
	int (*prime_fd_to_handle)(struct drm_device *dev, struct drm_file *file_priv,
				int prime_fd, uint32_t *handle);
	struct dma_buf * (*gem_prime_export)(struct drm_device *dev,
				struct drm_gem_object *obj, int flags);
	struct drm_gem_object * (*gem_prime_import)(struct drm_device *dev,
				struct dma_buf *dma_buf);
//	/* low-level interface used by drm_gem_prime_{import,export} */
//	int (*gem_prime_pin)(struct drm_gem_object *obj);
//	void (*gem_prime_unpin)(struct drm_gem_object *obj);
//	struct reservation_object * (*gem_prime_res_obj)(
//				struct drm_gem_object *obj);
	struct sg_table *(*gem_prime_get_sg_table)(struct drm_gem_object *obj);
	struct drm_gem_object *(*gem_prime_import_sg_table)(
				struct drm_device *dev,
				struct dma_buf_attachment *attach,
				struct sg_table *sgt);
	void *(*gem_prime_vmap)(struct drm_gem_object *obj);
	void (*gem_prime_vunmap)(struct drm_gem_object *obj, void *vaddr);
	int (*gem_prime_mmap)(struct drm_gem_object *obj,
				struct vm_area_struct *vma);

//	void (*vgaarb_irq)(struct drm_device *dev, bool state);

	int (*dumb_create)(struct drm_file *file_priv,
			   struct drm_device *dev,
			   struct drm_mode_create_dumb *args);
	int (*dumb_map_offset)(struct drm_file *file_priv,
			       struct drm_device *dev, uint32_t handle,
			       uint64_t *offset);
	int (*dumb_destroy)(struct drm_file *file_priv,
			    struct drm_device *dev,
			    uint32_t handle);

//	/* Driver private ops for this object */
//	const struct vm_operations_struct *gem_vm_ops;

	int major;
	int minor;
	int patchlevel;
	char *name;
	char *desc;
	char *date;

	u32 driver_features;
	int dev_priv_size;
//	const struct drm_ioctl_desc *ioctls;
//	int num_ioctls;
	const struct file_operations *fops;

//	/* List of devices hanging off this driver with stealth attach. */
//	struct list_head legacy_dev_list;
};



static inline
int drm_printk(const char *level, unsigned int category, const char *s, ...)
{
        return 0;
}

#define KERN_EMERG      ""    /* system is unusable */
#define KERN_ALERT      "1"    /* action must be taken immediately */
#define KERN_CRIT       "2"    /* critical conditions */
#define KERN_ERR        "3"    /* error conditions */
#define KERN_WARNING    "4"    /* warning conditions */
#define KERN_NOTICE     "5"    /* normal but significant condition */
#define KERN_INFO       "6"    /* informational */
#define KERN_DEBUG      "7"    /* debug-level messages */

#define DRM_UT_NONE		0x00
#define DRM_UT_CORE 		0x01
#define DRM_UT_DRIVER		0x02
#define DRM_UT_KMS		0x04
#define DRM_UT_PRIME		0x08
#define DRM_UT_ATOMIC		0x10
#define DRM_UT_VBL		0x20

#define DRM_DEBUG_KMS(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_KMS, fmt, ##__VA_ARGS__)

#define DRM_ERROR(fmt, ...)						\
	drm_printk(KERN_ERR, DRM_UT_NONE, fmt,	##__VA_ARGS__)



static inline void drm_crtc_force_disable_all(struct drm_device *dev)
{

}

// strncpy
#include <string.h>

static inline void drm_mode_convert_to_umode(struct drm_mode_modeinfo *out,
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

static inline uint32_t drm_mode_legacy_fb_format(uint32_t bpp, uint32_t depth)
{
        uint32_t fmt;

        switch (bpp) {
        case 8:
                fmt = DRM_FORMAT_C8;
                break;
        case 16:
                if (depth == 15)
                        fmt = DRM_FORMAT_XRGB1555;
                else
                        fmt = DRM_FORMAT_RGB565;
                break;
        case 24:
                fmt = DRM_FORMAT_RGB888;
                break;
        case 32:
                if (depth == 24)
                        fmt = DRM_FORMAT_XRGB8888;
                else if (depth == 30)
                        fmt = DRM_FORMAT_XRGB2101010;
                else
                        fmt = DRM_FORMAT_ARGB8888;
                break;
        default:
                DRM_ERROR("bad bpp, assuming x8r8g8b8 pixel format\n");
                fmt = DRM_FORMAT_XRGB8888;
                break;
        }

        return fmt;
}


#endif
