#ifndef __LINUX_GPIO_CONSUMER_H
#define __LINUX_GPIO_CONSUMER_H

struct gpio_desc {
//	struct gpio_device      *gdev;
//	unsigned long           flags;
///* flag symbols are bit numbers */
//#define FLAG_REQUESTED  0
//#define FLAG_IS_OUT     1
//#define FLAG_EXPORT     2       /* protected by sysfs_lock */
//#define FLAG_SYSFS      3       /* exported via /sys/class/gpio/control */
//#define FLAG_ACTIVE_LOW 6       /* value has active low */
//#define FLAG_OPEN_DRAIN 7       /* Gpio is open drain type */
//#define FLAG_OPEN_SOURCE 8      /* Gpio is open source type */
//#define FLAG_USED_AS_IRQ 9      /* GPIO is connected to an IRQ */
//#define FLAG_IS_HOGGED  11      /* GPIO is hogged */
//
//	/* Connection label */
//	const char              *label;
//	/* Name of the GPIO */
//	const char              *name;
};

static inline void gpiod_set_value_cansleep(struct gpio_desc *desc, int value)
{
}

#endif
