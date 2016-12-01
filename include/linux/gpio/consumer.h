#ifndef __LINUX_GPIO_CONSUMER_H
#define __LINUX_GPIO_CONSUMER_H

#include <linux/kernel.h>

// PATH_MAX
#include <linux/limits.h>

// printf
#include <stdio.h>

// file
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	int fd;
	int gpio;
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
	const char              *name;
};

static inline int file_write_string(const char *pathname, const char *buf)
{
	int fd, ret;

	fd = open(pathname, O_WRONLY);
	if (fd == -1) {
		printf("%s: Failed to open '%s': %s", __func__, pathname, strerror(errno));
		return -errno;
	}

	ret = write(fd, buf, strlen(buf));
	if (ret == -1) {
		printf("%s: Failed to write '%s': %s", __func__, pathname, strerror(errno));
		close(fd);
		return -errno;
	}

	close(fd);

	return 0;
}

static inline struct gpio_desc *
devm_gpiod_get_optional(struct device *dev, const char *con_id,
			  enum gpiod_flags flags)
{
	struct gpio_desc *desc;
	char fname[PATH_MAX];
	int gpio;

	if (!strncmp(con_id, "dc", 3))
		gpio = 24;
	else if (!strncmp(con_id, "reset", 6))
		gpio = 23;
	else if (!strncmp(con_id, "led", 4))
		gpio = 18;
	else
		return ERR_PTR(-EINVAL);

	DRM_DEBUG("name=%s -> %d\n", con_id, gpio);

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return ERR_PTR(-ENOMEM);

	desc->name = con_id;
	desc->gpio = gpio;

	snprintf(fname, sizeof(fname), "/sys/class/gpio/gpio%d/value", desc->gpio);

	if(access(fname, F_OK) == -1) {
		char buf[PATH_MAX];
		int ret;

		snprintf(buf, sizeof(buf), "%d", desc->gpio);
		ret = file_write_string("/sys/class/gpio/export", buf);
		if (ret)
			return ERR_PTR(ret);

		snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", desc->gpio);
		ret = file_write_string(buf, "out");
		if (ret)
			return ERR_PTR(ret);
	}

	desc->fd = open(fname, O_WRONLY);
	if (desc->fd == -1) {
		printf("%s: Failed to open '%s': %s", __func__, fname, strerror(errno));
		return ERR_PTR(-errno);
	}

	return desc;
}

static inline void gpiod_set_value_cansleep(struct gpio_desc *desc, int value)
{
	char buf[2];

	DRM_DEBUG("gpio=%d, value=%d\n", desc->gpio, value);

	snprintf(buf, sizeof(buf), "%d", value ? 1 : 0);
	if (write(desc->fd, buf, strlen(buf)) == -1)
		printf("%s(%d, %d): Failed to write gpio: %s\n",
		       __func__, desc->gpio, value, strerror(errno));
}

#endif
