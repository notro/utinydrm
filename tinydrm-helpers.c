/*
 * Copyright (C) 2016 Noralf Trønnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <drm/drmP.h>
#include <drm/tinydrm/tinydrm.h>
#include <drm/tinydrm/tinydrm-helpers.h>
//#include <linux/backlight.h>
//#include <linux/pm.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>

/**
 * tinydrm_merge_clips - merge clip rectangles
 * @dst: destination clip rectangle
 * @src: source clip rectangle(s)
 * @num_clips: number of @src clip rectangles
 * @flags: dirty fb ioctl flags
 * @max_width: maximum width of @dst
 * @max_height: maximum height of @dst
 *
 * This function merges @src clip rectangle(s) into @dst. If @src is NULL,
 * @max_width and @min_width is used to set a full @dst clip rectangle.
 */
void tinydrm_merge_clips(struct drm_clip_rect *dst,
			 struct drm_clip_rect *src, unsigned int num_clips,
			 unsigned int flags, u32 max_width, u32 max_height)
{
	unsigned int i;

	if (!src || !num_clips) {
		dst->x1 = 0;
		dst->x2 = max_width;
		dst->y1 = 0;
		dst->y2 = max_height;
		return;
	}

	dst->x1 = ~0;
	dst->y1 = ~0;
	dst->x2 = 0;
	dst->y2 = 0;

	for (i = 0; i < num_clips; i++) {
		if (flags & DRM_MODE_FB_DIRTY_ANNOTATE_COPY)
			i++;
		dst->x1 = min(dst->x1, src[i].x1);
		dst->x2 = max(dst->x2, src[i].x2);
		dst->y1 = min(dst->y1, src[i].y1);
		dst->y2 = max(dst->y2, src[i].y2);
	}

	if (dst->x2 > max_width || dst->y2 > max_height ||
	    dst->x1 >= dst->x2 || dst->y1 >= dst->y2) {
		DRM_DEBUG_KMS("Illegal clip: x1=%u, x2=%u, y1=%u, y2=%u\n",
			      dst->x1, dst->x2, dst->y1, dst->y2);
		dst->x1 = 0;
		dst->y1 = 0;
		dst->x2 = max_width;
		dst->y2 = max_height;
	}
}

/**
 * tinydrm_xrgb8888_to_rgb565 - convert xrgb8888 to rgb565
 * @src: xrgb8888 source buffer
 * @dst: rgb565 destination buffer
 * @num_pixels: number of pixels to copy
 *
 * Drivers can use this function for rgb565 devices that don't natively
 * support xrgb8888.
 */
void tinydrm_xrgb8888_to_rgb565(u32 *src, u16 *dst, unsigned int num_pixels)
{
	int i;

	for (i = 0; i < num_pixels; i++) {
		*dst = ((*src & 0x00F80000) >> 8) |
		       ((*src & 0x0000FC00) >> 5) |
		       ((*src & 0x000000F8) >> 3);
		src++;
		dst++;
	}
}

struct backlight_device *tinydrm_of_find_backlight(struct device *dev)
{
	struct gpio_desc *led;

	led = devm_gpiod_get_optional(dev, "led", GPIOD_OUT_LOW);
	if (IS_ERR(led)) {
		DRM_ERROR("Failed to get led gpio %ld\n", PTR_ERR(led));
		return ERR_PTR(PTR_ERR(led));
	}

	return (struct backlight_device *)led;
}

/**
 * tinydrm_enable_backlight - enable backlight helper
 * @backlight: backlight device
 *
 * Helper to enable backlight for use in &tinydrm_funcs ->enable callback
 * functions.
 */
int tinydrm_enable_backlight(struct backlight_device *backlight)
{
	if (backlight)
		gpiod_set_value_cansleep((struct gpio_desc *)backlight, 1);

	return 0;
}

/**
 * tinydrm_disable_backlight - disable backlight helper
 * @backlight: backlight device
 *
 * Helper to disable backlight for use in &tinydrm_funcs ->disable callback
 * functions.
 */
void tinydrm_disable_backlight(struct backlight_device *backlight)
{
	if (backlight)
		gpiod_set_value_cansleep((struct gpio_desc *)backlight, 0);
}

/*
 * tinydrm_simple_pm_ops - tinydrm simple power management operations
 *
 * This provides simple suspend/resume power management and can be assigned
 * to the drivers &device_driver->pm property. &tinydrm_device must be set
 * on the device using dev_set_drvdata() or equivalent.
 */
const struct dev_pm_ops tinydrm_simple_pm_ops = {
};

/**
 * tinydrm_spi_shutdown - SPI driver shutdown callback helper
 * @spi: SPI device
 *
 * This is a helper function for the &spi_driver ->shutdown callback which
 * makes sure that the tinydrm device is disabled and unprepared on shutdown.
 * &tinydrm_device must be set on the device using spi_set_drvdata().
 */
void tinydrm_spi_shutdown(struct spi_device *spi)
{
	struct tinydrm_device *tdev = spi_get_drvdata(spi);

	if (tdev)
		tinydrm_shutdown(tdev);
}
