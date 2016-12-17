#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <drm/drmP.h>
#include <drm/tinydrm/tinydrm.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

static int spi_ioc_message(struct spi_ioc_transfer *utr, unsigned int num)
{
	int fd, ret;

	fd = open("/dev/spidev0.0", O_RDWR);
	if (fd < 0) {
		DRM_ERROR("Failed to open spidev\n");
		return -errno;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(num), utr);
	if (ret < 1) {
		printf("%s: Failed to ioctl spidev: %s", __func__, strerror(errno));
		ret = -errno;
	}

	close(fd);

	return ret < 0 ? ret : 0;
}

static int spi_buf_transfer(struct spi_transfer *tr, int buf_fd)
{
	struct spi_ioc_transfer utr = {
		.tx_dma_fd = buf_fd,
		.speed_hz = tr->speed_hz,
		//.bits_per_word = tr->bits_per_word,
		.bits_per_word = 8,
	};
	size_t chunk, max_chunk = 320 * 240 * 2 / 5; // 30720
	size_t len = tr->len;
	u32 offset = 0;
	int ret;

	DRM_DEBUG("%s: buf_fd=%d, tr->len=%u\n", __func__, buf_fd, tr->len);

	while (len) {
		chunk = min(len, max_chunk);

		utr.dma_offset = offset;
		utr.len = chunk;

		offset += chunk;
		len -= chunk;

		ret = spi_ioc_message(&utr, 1);
		if (ret)
			return ret;
	}

	return 0;
}

int spi_sync(struct spi_device *spi, struct spi_message *m)
{
	struct spi_transfer *tr = list_first_entry(&m->transfers, struct spi_transfer, transfer_list);
	struct spi_transfer *tr2 = NULL;
	struct spi_ioc_transfer utr[2] = {
		{
			.tx_buf = (unsigned long)tr->tx_buf,
			.len = tr->len,
			.speed_hz = tr->speed_hz,
			//.bits_per_word = tr->bits_per_word,
			.bits_per_word = 8,
		},
	};
	struct tinydrm_device *tdev;
	struct utinydrm_fb *ufb;
	struct utinydrm *udev;
	int fd, ret;

	DRM_DEBUG("message=%p, tr=%p\n", m, tr);

	tdev = spi_get_drvdata(spi);
	udev = &tdev->drm.udev;

	for (ufb = udev->fbs; ufb != NULL; ufb = ufb->next) {
		if (tr->tx_buf >= ufb->map && tr->tx_buf < (ufb->map + ufb->map_size))
			break;
	}

	if (ufb && udev->buf_fd >= 0) {
		if (!tr->speed_hz)
			tr->speed_hz = spi->max_speed_hz;

		return spi_buf_transfer(tr, udev->buf_fd);

	} else if (ufb) {
		DRM_DEBUG("[FB:%u] ufb->map=%p, tr->tx_buf=%p\n", ufb->id, ufb->map, tr->tx_buf);
		utr[0].tx_buf = 0;
		utr[0].tx_dma_fd = ufb->buf_fd;
		utr[0].dma_offset = (u32)(tr->tx_buf - ufb->map);

	} else if (tx_buf) {
//		struct dma_buf_sync sync_args = {
//			.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_WRITE, // DMA_BUF_SYNC_RW -> DMA_BIDIRECTIONAL
//		};
//
//		ret = ioctl(tx_buf->fd, DMA_BUF_IOCTL_SYNC, &sync_args);
//		if (ret == -1)
//			perror("Failed to DMA_BUF_SYNC_END");

		if (tr->tx_buf >= tx_buf->map && tr->tx_buf < (tx_buf->map + tx_buf->map_size)) {
			DRM_DEBUG("%s: tx_buf->map=%p, tx_buf->map_size=%zu\n", __func__, tx_buf->map, tx_buf->map_size);
			utr[0].tx_buf = 0;
			utr[0].tx_dma_fd = tx_buf->fd;
			utr[0].dma_offset = (u32)(tr->tx_buf - tx_buf->map);
		}
	}

	if (!utr[0].speed_hz)
		utr[0].speed_hz = spi->max_speed_hz;

	if (!list_is_singular(&m->transfers)) {
		tr2 = list_next_entry(tr, transfer_list);

		utr[1].rx_buf = (unsigned long)tr2->rx_buf;
		utr[1].len = tr2->len;
		utr[1].speed_hz = tr2->speed_hz;
		//utr[1].bits_per_word = tr2->bits_per_word;
		utr[1].bits_per_word = 8;

		DRM_DEBUG("message=%p, tr2=%p, len=%u\n", m, tr2, tr2->len);
	}


	fd = open("/dev/spidev0.0", O_RDWR);
	if (fd < 0) {
		DRM_ERROR("Failed to open spidev\n");
		return -errno;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(tr2 ? 2 : 1), utr);
	if (ret < 1) {
		printf("%s: Failed to ioctl spidev: %s", __func__, strerror(errno));
		ret = -errno;
		close(fd);
		return ret;
	}

	close(fd);

	return 0;
}
