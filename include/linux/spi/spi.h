#ifndef __LINUX_SPI_H
#define __LINUX_SPI_H

#include <linux/kernel.h>

// memset
#include <string.h>

struct spi_device;
struct spi_transfer;

struct spi_master {
	struct device	dev;

//	struct list_head list;

//	s16			bus_num;
//	u16			num_chipselect;
//	u16			dma_alignment;
//	u16			mode_bits;

	u32			bits_per_word_mask;
#define SPI_BPW_MASK(bits) BIT((bits) - 1)
#define SPI_BIT_MASK(bits) (((bits) == 32) ? ~0U : (BIT(bits) - 1))
#define SPI_BPW_RANGE_MASK(min, max) (SPI_BIT_MASK(max) - SPI_BIT_MASK(min - 1))

//	u32			min_speed_hz;
//	u32			max_speed_hz;

//	u16			flags;
//#define SPI_MASTER_HALF_DUPLEX	BIT(0)		/* can't do full duplex */
//#define SPI_MASTER_NO_RX	BIT(1)		/* can't do buffer read */
//#define SPI_MASTER_NO_TX	BIT(2)		/* can't do buffer write */
//#define SPI_MASTER_MUST_RX      BIT(3)		/* requires rx */
//#define SPI_MASTER_MUST_TX      BIT(4)		/* requires tx */
#if 0
	size_t (*max_transfer_size)(struct spi_device *spi);
	size_t (*max_message_size)(struct spi_device *spi);

	struct mutex		io_mutex;
	spinlock_t		bus_lock_spinlock;
	struct mutex		bus_lock_mutex;
	bool			bus_lock_flag;
	int			(*setup)(struct spi_device *spi);
	int			(*transfer)(struct spi_device *spi,
						struct spi_message *mesg);
	void			(*cleanup)(struct spi_device *spi);
#endif
	bool			(*can_dma)(struct spi_master *master,
					   struct spi_device *spi,
					   struct spi_transfer *xfer);
#if 0
	bool				queued;
	struct kthread_worker		kworker;
	struct task_struct		*kworker_task;
	struct kthread_work		pump_messages;
	spinlock_t			queue_lock;
	struct list_head		queue;
	struct spi_message		*cur_msg;
	bool				idling;
	bool				busy;
	bool				running;
	bool				rt;
	bool				auto_runtime_pm;
	bool                            cur_msg_prepared;
	bool				cur_msg_mapped;
	struct completion               xfer_completion;
#endif
	size_t				max_dma_len;
#if 0
	int (*prepare_transfer_hardware)(struct spi_master *master);
	int (*transfer_one_message)(struct spi_master *master,
				    struct spi_message *mesg);
	int (*unprepare_transfer_hardware)(struct spi_master *master);
	int (*prepare_message)(struct spi_master *master,
			       struct spi_message *message);
	int (*unprepare_message)(struct spi_master *master,
				 struct spi_message *message);
	int (*spi_flash_read)(struct  spi_device *spi,
			      struct spi_flash_read_message *msg);
	bool (*flash_read_supported)(struct spi_device *spi);

	/*
	 * These hooks are for drivers that use a generic implementation
	 * of transfer_one_message() provied by the core.
	 */
	void (*set_cs)(struct spi_device *spi, bool enable);
	int (*transfer_one)(struct spi_master *master, struct spi_device *spi,
			    struct spi_transfer *transfer);
	void (*handle_err)(struct spi_master *master,
			   struct spi_message *message);

	/* gpio chip select */
	int			*cs_gpios;

	/* statistics */
	struct spi_statistics	statistics;

	/* DMA channels for use with core dmaengine helpers */
	struct dma_chan		*dma_tx;
	struct dma_chan		*dma_rx;

	/* dummy data for full duplex devices */
	void			*dummy_rx;
	void			*dummy_tx;

	int (*fw_translate_cs)(struct spi_master *master, unsigned cs);
#endif
};

struct spi_device {
	struct device		dev;
	struct spi_master	*master;
	struct spi_master 			master_instance;
	u32			max_speed_hz;
	u8			chip_select;
	u8			bits_per_word;
	u16			mode;
#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */
#define	SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define	SPI_MODE_1	(0|SPI_CPHA)
#define	SPI_MODE_2	(SPI_CPOL|0)
#define	SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define	SPI_CS_HIGH	0x04			/* chipselect active high? */
#define	SPI_LSB_FIRST	0x08			/* per-word bits-on-wire */
#define	SPI_3WIRE	0x10			/* SI/SO signals shared */
#define	SPI_LOOP	0x20			/* loopback mode */
#define	SPI_NO_CS	0x40			/* 1 dev/bus, no chipselect */
#define	SPI_READY	0x80			/* slave pulls low to pause */
#define	SPI_TX_DUAL	0x100			/* transmit with 2 wires */
#define	SPI_TX_QUAD	0x200			/* transmit with 4 wires */
#define	SPI_RX_DUAL	0x400			/* receive with 2 wires */
#define	SPI_RX_QUAD	0x800			/* receive with 4 wires */
#if 0
	int			irq;
	void			*controller_state;
	void			*controller_data;
	char			modalias[SPI_NAME_SIZE];
	int			cs_gpio;	/* chip select gpio */

	/* the statistics */
	struct spi_statistics	statistics;
#endif
};

struct spi_driver {
	const struct spi_device_id *id_table;
	int			(*probe)(struct spi_device *spi);
//	int			(*remove)(struct spi_device *spi);
	void			(*shutdown)(struct spi_device *spi);
	struct device_driver	driver;
};

#define SPI_NAME_SIZE   32

struct spi_device_id {
	char name[SPI_NAME_SIZE];
	//kernel_ulong_t driver_data;
	unsigned long driver_data;
};

extern struct spi_driver *driver;

#define module_spi_driver(sdrv) struct spi_driver *driver = &sdrv

static inline int spi_add_device(struct spi_device *spi)
{
	spi->bits_per_word = 8;
	spi->master = &spi->master_instance;

	return 0;
}

struct spi_transfer {
	const void	*tx_buf;
	void		*rx_buf;
	unsigned	len;
#if 0
	dma_addr_t	tx_dma;
	dma_addr_t	rx_dma;
	struct sg_table tx_sg;
	struct sg_table rx_sg;

	unsigned	cs_change:1;
	unsigned	tx_nbits:3;
	unsigned	rx_nbits:3;
#define	SPI_NBITS_SINGLE	0x01 /* 1bit transfer */
#define	SPI_NBITS_DUAL		0x02 /* 2bits transfer */
#define	SPI_NBITS_QUAD		0x04 /* 4bits transfer */
#endif
	u8		bits_per_word;
//	u16		delay_usecs;
	u32		speed_hz;

	struct list_head transfer_list;
};

struct spi_message {
	struct list_head	transfers;
#if 0
	struct spi_device	*spi;
	unsigned		is_dma_mapped:1;
	void			(*complete)(void *context);
	void			*context;
	unsigned		frame_length;
	unsigned		actual_length;
	int			status;
	struct list_head	queue;
	void			*state;
	struct list_head        resources;
#endif
};

static inline void spi_message_init_no_memset(struct spi_message *m)
{
	INIT_LIST_HEAD(&m->transfers);
//	INIT_LIST_HEAD(&m->resources);
}

static inline void spi_message_init(struct spi_message *m)
{
	memset(m, 0, sizeof *m);
	spi_message_init_no_memset(m);
}

static inline void
spi_message_add_tail(struct spi_transfer *t, struct spi_message *m)
{
	list_add_tail(&t->transfer_list, &m->transfers);
}

static inline void
spi_message_init_with_transfers(struct spi_message *m,
struct spi_transfer *xfers, unsigned int num_xfers)
{
	unsigned int i;

	spi_message_init(m);
	for (i = 0; i < num_xfers; ++i)
		spi_message_add_tail(&xfers[i], m);
}

int spi_sync(struct spi_device *spi, struct spi_message *message);

static inline void spi_set_drvdata(struct spi_device *spi, void *data)
{
	dev_set_drvdata(&spi->dev, data);
}

static inline void *spi_get_drvdata(struct spi_device *spi)
{
	return dev_get_drvdata(&spi->dev);
}

static inline size_t
spi_max_transfer_size(struct spi_device *spi)
{
	/* spi-bcm2835 */
	return SIZE_MAX;
}

#endif
