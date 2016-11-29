#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define MODULE_LICENSE(_license)
#define EXPORT_SYMBOL(_mod)
#define EXPORT_SYMBOL_GPL(_mod)

#define IS_ENABLED(x)	1

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

#define swap(a, b) \
        do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define ___constant_swab16(x) ((__u16)(                         \
        (((__u16)(x) & (__u16)0x00ffU) << 8) |                  \
        (((__u16)(x) & (__u16)0xff00U) >> 8)))
#define swab16 ___constant_swab16

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

#define GFP_KERNEL 0
#define kmalloc_array(num, size, flag)	calloc(num, size)
#define kfree(x) free(x)

struct vm_area_struct {

};

struct list_head {
	struct list_head *next, *prev;
};

#include "list.h"

struct sg_table {

};

struct device {
	void *driver_data;
};

static inline void *dev_get_drvdata(const struct device *dev)
{
        return dev->driver_data;
}

static inline void dev_set_drvdata(struct device *dev, void *data)
{
        dev->driver_data = data;
}

struct file_operations {

};

struct dev_pm_ops {

};

struct mutex {

};

#define mutex_lock(lock)
#define mutex_unlock(lock)


#define pr_debug printf

static inline void dev_err(const struct device *dev, const char *fmt, ...)
{}


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

#endif
