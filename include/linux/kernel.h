#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <stddef.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define MODULE_LICENSE(_license)
#define EXPORT_SYMBOL(_mod)
#define EXPORT_SYMBOL_GPL(_mod)

#include <stdbool.h>

#define u32 uint32_t
#define u16 uint16_t

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

#define swap(a, b) \
        do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


struct vm_area_struct {

};

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

#endif
