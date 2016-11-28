#ifndef __LINUX_SPI_H
#define __LINUX_SPI_H

#include <linux/kernel.h>

struct spi_device {
	struct device dev;
};

static inline void spi_set_drvdata(struct spi_device *spi, void *data)
{
	dev_set_drvdata(&spi->dev, data);
}

static inline void *spi_get_drvdata(struct spi_device *spi)
{
	return dev_get_drvdata(&spi->dev);
}

#endif
