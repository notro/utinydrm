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

struct vm_area_struct {

};

struct sg_table {

};

struct device {

};

struct file_operations {

};


#endif
