#ifndef __LINUX_GPIO_CONSUMER_H
#define __LINUX_GPIO_CONSUMER_H

#include <linux/kernel.h>

#define GPIOD_FLAGS_BIT_DIR_SET		BIT(0)
#define GPIOD_FLAGS_BIT_DIR_OUT		BIT(1)
#define GPIOD_FLAGS_BIT_DIR_VAL		BIT(2)

enum gpiod_flags {
	GPIOD_ASIS	= 0,
	GPIOD_IN	= GPIOD_FLAGS_BIT_DIR_SET,
	GPIOD_OUT_LOW	= GPIOD_FLAGS_BIT_DIR_SET | GPIOD_FLAGS_BIT_DIR_OUT,
	GPIOD_OUT_HIGH	= GPIOD_FLAGS_BIT_DIR_SET | GPIOD_FLAGS_BIT_DIR_OUT |
			  GPIOD_FLAGS_BIT_DIR_VAL,
};

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

static inline struct gpio_desc *
devm_gpiod_get_optional(struct device *dev, const char *con_id,
			  enum gpiod_flags flags)
{
	DRM_DEBUG("name=%s\n", con_id);

	return NULL;
}

static inline void gpiod_set_value_cansleep(struct gpio_desc *desc, int value)
{
	DRM_DEBUG("value=%d\n", value);
}

#endif
