#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

// usleep
#include <unistd.h>

#include <linux/types.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define MODULE_DEVICE_TABLE(type, name)
#define MODULE_AUTHOR(desc)
#define MODULE_DESCRIPTION(desc)
#define MODULE_LICENSE(_license)
#define EXPORT_SYMBOL(_mod)
#define EXPORT_SYMBOL_GPL(_mod)

#define IS_ENABLED(x)	1

#define WARN_ON_ONCE(x) (x)

#define unlikely(x) (x)

#define ENOTSUPP        524     /* Operation is not supported */

#define __must_check            __attribute__((warn_unused_result))

#include <stdbool.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#define BIT(nr)                 (1UL << (nr))

#define __must_be_array(a)      0
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define min(x, y) ({                            \
        typeof(x) _min1 = (x);                  \
        typeof(y) _min2 = (y);                  \
        (void) (&_min1 == &_min2);              \
        _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                            \
        typeof(x) _max1 = (x);                  \
        typeof(y) _max2 = (y);                  \
        (void) (&_max1 == &_max2);              \
        _max1 > _max2 ? _max1 : _max2; })

#define min_t(type, x, y) ({                    \
        type __min1 = (x);                      \
        type __min2 = (y);                      \
        __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({                    \
        type __max1 = (x);                      \
        type __max2 = (y);                      \
        __max1 > __max2 ? __max1: __max2; })

#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)
#define clamp_val(val, lo, hi) clamp_t(typeof(val), val, lo, hi)

#define swap(a, b) \
        do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define ___constant_swab16(x) ((__u16)(                         \
        (((__u16)(x) & (__u16)0x00ffU) << 8) |                  \
        (((__u16)(x) & (__u16)0xff00U) >> 8)))
#define swab16 ___constant_swab16

#define ___constant_swab64(x) ((__u64)(                         \
        (((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) |   \
        (((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) |   \
        (((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) |   \
        (((__u64)(x) & (__u64)0x00000000ff000000ULL) <<  8) |   \
        (((__u64)(x) & (__u64)0x000000ff00000000ULL) >>  8) |   \
        (((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) |   \
        (((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) |   \
        (((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56)))

#define cpu_to_be64 ___constant_swab64

static inline u16 __get_unaligned_le16(const u8 *p)
{
        return p[0] | p[1] << 8;
}

static inline u32 __get_unaligned_le32(const u8 *p)
{
        return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

static inline u64 __get_unaligned_le64(const u8 *p)
{
        return (u64)__get_unaligned_le32(p + 4) << 32 |
               __get_unaligned_le32(p);
}

static inline u16 get_unaligned_le16(const void *p)
{
        return __get_unaligned_le16((const u8 *)p);
}

static inline u32 get_unaligned_le32(const void *p)
{
        return __get_unaligned_le32((const u8 *)p);
}

static inline u64 get_unaligned_le64(const void *p)
{
        return __get_unaligned_le64((const u8 *)p);
}

extern void __bad_unaligned_access_size(void);

#define __get_unaligned_le(ptr) ({                     \
        __builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),                      \
        __builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_le16((ptr)),   \
        __builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_le32((ptr)),   \
        __builtin_choose_expr(sizeof(*(ptr)) == 8, get_unaligned_le64((ptr)),   \
        __bad_unaligned_access_size()))));                                      \
        })

#define get_unaligned  __get_unaligned_le

static inline bool is_power_of_2(unsigned long n)
{
        return (n != 0 && ((n & (n - 1)) == 0));
}

#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP

#define IS_ALIGNED(x, a)                (((x) & ((typeof(x))(a) - 1)) == 0)

static __always_inline int fls(int x)
{
        int r = 32;

        if (!x)
                return 0;
        if (!(x & 0xffff0000u)) {
                x <<= 16;
                r -= 16;
        }
        if (!(x & 0xff000000u)) {
                x <<= 8;
                r -= 8;
        }
        if (!(x & 0xf0000000u)) {
                x <<= 4;
                r -= 4;
        }
        if (!(x & 0xc0000000u)) {
                x <<= 2;
                r -= 2;
        }
        if (!(x & 0x80000000u)) {
                x <<= 1;
                r -= 1;
        }
        return r;
}

static inline __attribute__((const))
int __ilog2_u32(u32 n)
{
	return fls(n) - 1;
}

#define ilog2(n)				\
(						\
	__ilog2_u32(n) 			\
 )

#define GFP_KERNEL 0
#define kmalloc_array(num, size, flag)	calloc(num, size)
#define kmalloc(size, flag)	malloc(size)
#define kzalloc(size, flag)	calloc(1, size)
#define kfree(x) free(x)
#define devm_kmalloc(dev, size, flag)	malloc(size)
#define devm_kzalloc(dev, size, flag)	calloc(1, size)

struct vm_area_struct {

};

struct list_head {
	struct list_head *next, *prev;
};

#include "list.h"

struct sg_table {

};

struct device {
	const char      *init_name;
	void            *driver_data;
	u64		coherent_dma_mask;
	void *utinydrm;
};

static inline const char *dev_name(const struct device *dev)
{
	return dev->init_name;
}

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

static inline int dma_coerce_mask_and_coherent(struct device *dev, u64 mask)
{
	return 0;
}

static inline void *dev_get_drvdata(const struct device *dev)
{
        return dev->driver_data;
}

static inline void dev_set_drvdata(struct device *dev, void *data)
{
        dev->driver_data = data;
}

struct of_device_id {
//	char    name[32];
//	char    type[32];
	char    compatible[128];
//	const void *data;
};

struct device_driver {
	const char		*name;
//	struct bus_type		*bus;

	struct module		*owner;
//	const char		*mod_name;	/* used for built-in modules */

//	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
//	enum probe_type probe_type;

	const struct of_device_id	*of_match_table;
//	const struct acpi_device_id	*acpi_match_table;

//	int (*probe) (struct device *dev);
//	int (*remove) (struct device *dev);
//	void (*shutdown) (struct device *dev);
//	int (*suspend) (struct device *dev, pm_message_t state);
//	int (*resume) (struct device *dev);
//	const struct attribute_group **groups;

	const struct dev_pm_ops *pm;

//	struct driver_private *p;
};

static inline bool device_property_read_bool(struct device *dev,
                                             const char *propname)
{
	return false;
}

static inline int device_property_read_u32(struct device *dev,
                                           const char *propname, u32 *val)
{
	return 0;
}

struct file_operations {

};

struct dev_pm_ops {

};

struct mutex {

};

#define mutex_lock(lock)
#define mutex_unlock(lock)

struct work_struct {

};

static inline void schedule_work(struct work_struct *work)
{
}

#define THIS_MODULE	NULL

#define KERN_EMERG      0    /* system is unusable */
#define KERN_ALERT      1    /* action must be taken immediately */
#define KERN_CRIT       2    /* critical conditions */
#define KERN_ERR        3    /* error conditions */
#define KERN_WARNING    4    /* warning conditions */
#define KERN_NOTICE     5    /* normal but significant condition */
#define KERN_INFO       6    /* informational */
#define KERN_DEBUG      7    /* debug-level messages */

extern int printk_level;

#define printk(level, fmt, ...)						\
({									\
	struct timespec __ts;						\
									\
	clock_gettime(CLOCK_MONOTONIC, &__ts);				\
	if (level <= printk_level)					\
		printf("[%ld.%09ld] " fmt, __ts.tv_sec, __ts.tv_nsec, ##__VA_ARGS__);	\
})

#define pr_debug(fmt, ...)			\
        printk(KERN_DEBUG, fmt, ##__VA_ARGS__)

#define dev_dbg(dev, fmt, ...)			\
        printk(KERN_DEBUG, fmt, ##__VA_ARGS__)


#define dev_err(dev, fmt, ...)			\
        printk(KERN_ERR, fmt, ##__VA_ARGS__)

#define dev_warn(dev, fmt, ...)			\
        printk(KERN_WARNING, fmt, ##__VA_ARGS__)

#define dev_level_once(dev_level, dev, fmt, ...)                        \
do {                                                                    \
        static bool __print_once;                         \
                                                                        \
        if (!__print_once) {                                            \
                __print_once = true;                                    \
                dev_level(dev, fmt, ##__VA_ARGS__);                     \
        }                                                               \
} while (0)

#define dev_err_once(dev, fmt, ...)                                     \
        dev_level_once(dev_err, dev, fmt, ##__VA_ARGS__)

#define dev_warn_once dev_err_once

#define msleep(x) usleep(x * 1000)


#define MAX_ERRNO       4095

#define IS_ERR_VALUE(x) unlikely((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void * ERR_PTR(long error)
{
	return (void *) error;
}

static inline long __must_check PTR_ERR(__force const void *ptr)
{
	return (long) ptr;
}

static inline bool __must_check IS_ERR(__force const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

#endif
