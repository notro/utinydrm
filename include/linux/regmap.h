#ifndef __LINUX_REGMAP_H
#define __LINUX_REGMAP_H

enum regcache_type {
	REGCACHE_NONE,
	REGCACHE_RBTREE,
	REGCACHE_COMPRESSED,
	REGCACHE_FLAT,
};

enum regmap_endian {
	/* Unspecified -> 0 -> Backwards compatible default */
	REGMAP_ENDIAN_DEFAULT = 0,
	REGMAP_ENDIAN_BIG,
	REGMAP_ENDIAN_LITTLE,
	REGMAP_ENDIAN_NATIVE,
};

struct regmap;

struct regmap_format {
	size_t buf_size;
	size_t reg_bytes;
	size_t pad_bytes;
	size_t val_bytes;
	void (*format_write)(struct regmap *map,
			     unsigned int reg, unsigned int val);
	void (*format_reg)(void *buf, unsigned int reg, unsigned int shift);
	void (*format_val)(void *buf, unsigned int val, unsigned int shift);
	unsigned int (*parse_val)(const void *buf);
	void (*parse_inplace)(void *buf);
};

struct regmap {
#if 0
	union {
		struct mutex mutex;
		struct {
			spinlock_t spinlock;
			unsigned long spinlock_flags;
		};
	};
	regmap_lock lock;
	regmap_unlock unlock;
	void *lock_arg; /* This is passed to lock/unlock functions */
	gfp_t alloc_flags;
#endif
	struct device *dev; /* Device we do I/O on */
	void *work_buf;     /* Scratch buffer used to format I/O */
	struct regmap_format format;  /* Buffer format */
	const struct regmap_bus *bus;
	void *bus_context;
#if 0
	const char *name;

	bool async;
	spinlock_t async_lock;
	wait_queue_head_t async_waitq;
	struct list_head async_list;
	struct list_head async_free;
	int async_ret;

	unsigned int max_register;
	bool (*writeable_reg)(struct device *dev, unsigned int reg);
	bool (*readable_reg)(struct device *dev, unsigned int reg);
	bool (*volatile_reg)(struct device *dev, unsigned int reg);
	bool (*precious_reg)(struct device *dev, unsigned int reg);
	const struct regmap_access_table *wr_table;
	const struct regmap_access_table *rd_table;
	const struct regmap_access_table *volatile_table;
	const struct regmap_access_table *precious_table;
#endif
	int (*reg_read)(void *context, unsigned int reg, unsigned int *val);
	int (*reg_write)(void *context, unsigned int reg, unsigned int val);
//	int (*reg_update_bits)(void *context, unsigned int reg,
//			       unsigned int mask, unsigned int val);
//
//	bool defer_caching;
	unsigned long read_flag_mask;
	unsigned long write_flag_mask;

	/* number of bits to (left) shift the reg value when formatting*/
	int reg_shift;
	int reg_stride;
	int reg_stride_order;

	/* regcache specific members */
//	const struct regcache_ops *cache_ops;
	enum regcache_type cache_type;
#if 0
	/* number of bytes in reg_defaults_raw */
	unsigned int cache_size_raw;
	/* number of bytes per word in reg_defaults_raw */
	unsigned int cache_word_size;
	/* number of entries in reg_defaults */
	unsigned int num_reg_defaults;
	/* number of entries in reg_defaults_raw */
	unsigned int num_reg_defaults_raw;

	/* if set, only the cache is modified not the HW */
	bool cache_only;
	/* if set, only the HW is modified not the cache */
	bool cache_bypass;
	/* if set, remember to free reg_defaults_raw */
	bool cache_free;

	struct reg_default *reg_defaults;
	const void *reg_defaults_raw;
	void *cache;
	/* if set, the cache contains newer data than the HW */
	bool cache_dirty;
	/* if set, the HW registers are known to match map->reg_defaults */
	bool no_sync_defaults;

	struct reg_sequence *patch;
	int patch_regs;
#endif
	/* if set, converts bulk read to single read */
	bool use_single_read;
	/* if set, converts bulk read to single read */
	bool use_single_write;
	/* if set, the device supports multi write mode */
	bool can_multi_write;

	/* if set, raw reads/writes are limited to this size */
	size_t max_raw_read;
	size_t max_raw_write;

//	struct rb_root range_tree;
//	void *selector_work_buf;	/* Scratch buffer used for selector */
};

//struct regmap_async;

typedef int (*regmap_hw_write)(void *context, const void *data,
			       size_t count);
typedef int (*regmap_hw_gather_write)(void *context,
				      const void *reg, size_t reg_len,
				      const void *val, size_t val_len);
//typedef int (*regmap_hw_async_write)(void *context,
//				     const void *reg, size_t reg_len,
//				     const void *val, size_t val_len,
//				     struct regmap_async *async);
typedef int (*regmap_hw_read)(void *context,
			      const void *reg_buf, size_t reg_size,
			      void *val_buf, size_t val_size);
//typedef int (*regmap_hw_reg_read)(void *context, unsigned int reg,
//				  unsigned int *val);
typedef int (*regmap_hw_reg_write)(void *context, unsigned int reg,
				   unsigned int val);
//typedef int (*regmap_hw_reg_update_bits)(void *context, unsigned int reg,
//					 unsigned int mask, unsigned int val);
//typedef struct regmap_async *(*regmap_hw_async_alloc)(void);
//typedef void (*regmap_hw_free_context)(void *context);

struct regmap_bus {
//	bool fast_io;
	regmap_hw_write write;
	regmap_hw_gather_write gather_write;
//	regmap_hw_async_write async_write;
	regmap_hw_reg_write reg_write;
//	regmap_hw_reg_update_bits reg_update_bits;
	regmap_hw_read read;
//	regmap_hw_reg_read reg_read;
//	regmap_hw_free_context free_context;
//	regmap_hw_async_alloc async_alloc;
//	u8 read_flag_mask;
	enum regmap_endian reg_format_endian_default;
	enum regmap_endian val_format_endian_default;
	size_t max_raw_read;
	size_t max_raw_write;
};

struct regmap_config {
//	const char *name;

	int reg_bits;
	int reg_stride;
	int pad_bits;
	int val_bits;

//	bool (*writeable_reg)(struct device *dev, unsigned int reg);
//	bool (*readable_reg)(struct device *dev, unsigned int reg);
//	bool (*volatile_reg)(struct device *dev, unsigned int reg);
//	bool (*precious_reg)(struct device *dev, unsigned int reg);
//	regmap_lock lock;
//	regmap_unlock unlock;
//	void *lock_arg;

//	int (*reg_read)(void *context, unsigned int reg, unsigned int *val);
//	int (*reg_write)(void *context, unsigned int reg, unsigned int val);

//	bool fast_io;

//	unsigned int max_register;
//	const struct regmap_access_table *wr_table;
//	const struct regmap_access_table *rd_table;
//	const struct regmap_access_table *volatile_table;
//	const struct regmap_access_table *precious_table;
//	const struct reg_default *reg_defaults;
//	unsigned int num_reg_defaults;
	enum regcache_type cache_type;
//	const void *reg_defaults_raw;
//	unsigned int num_reg_defaults_raw;

//	unsigned long read_flag_mask;
//	unsigned long write_flag_mask;

	bool use_single_rw;
	bool can_multi_write;

//	enum regmap_endian reg_format_endian;
//	enum regmap_endian val_format_endian;

//	const struct regmap_range_cfg *ranges;
//	unsigned int num_ranges;
};

static inline enum regmap_endian regmap_get_reg_endian(const struct regmap_bus *bus,
					const struct regmap_config *config)
{
	enum regmap_endian endian;

	/* Retrieve the endianness specification from the bus config */
	if (bus && bus->reg_format_endian_default)
		endian = bus->reg_format_endian_default;

	/* If the bus specified a non-default value, use that */
	if (endian != REGMAP_ENDIAN_DEFAULT)
		return endian;

	/* Use this if no other value was found */
	return REGMAP_ENDIAN_BIG;
}

static inline enum regmap_endian regmap_get_val_endian(struct device *dev,
					 const struct regmap_bus *bus,
					 const struct regmap_config *config)
{
	enum regmap_endian endian;

	/* Retrieve the endianness specification from the bus config */
	if (bus && bus->val_format_endian_default)
		endian = bus->val_format_endian_default;

	/* If the bus specified a non-default value, use that */
	if (endian != REGMAP_ENDIAN_DEFAULT)
		return endian;

	/* Use this if no other value was found */
	return REGMAP_ENDIAN_BIG;
}

static inline void regmap_set_work_buf_flag_mask(struct regmap *map, int max_bytes,
					  unsigned long mask)
{
	u8 *buf;
	int i;

	if (!mask || !map->work_buf)
		return;

	buf = map->work_buf;

	for (i = 0; i < max_bytes; i++)
		buf[i] |= (mask >> (8 * i)) & 0xff;
}

static inline int _regmap_raw_write(struct regmap *map, unsigned int reg,
		      const void *val, size_t val_len)
{
	//unsigned long flags;
	void *work_val = map->work_buf + map->format.reg_bytes +
		map->format.pad_bytes;
	void *buf;
	int ret = -ENOTSUPP;
	size_t len;
	//int i;

	map->format.format_reg(map->work_buf, reg, map->reg_shift);
	regmap_set_work_buf_flag_mask(map, map->format.reg_bytes,
				      map->write_flag_mask);

	/*
	 * Essentially all I/O mechanisms will be faster with a single
	 * buffer to write.  Since register syncs often generate raw
	 * writes of single registers optimise that case.
	 */
	if (val != work_val && val_len == map->format.val_bytes) {
		memcpy(work_val, val, map->format.val_bytes);
		val = work_val;
	}

	/* If we're doing a single register write we can probably just
	 * send the work_buf directly, otherwise try to do a gather
	 * write.
	 */
	if (val == work_val)
		ret = map->bus->write(map->bus_context, map->work_buf,
				      map->format.reg_bytes +
				      map->format.pad_bytes +
				      val_len);
	else if (map->bus->gather_write)
		ret = map->bus->gather_write(map->bus_context, map->work_buf,
					     map->format.reg_bytes +
					     map->format.pad_bytes,
					     val, val_len);

	/* If that didn't work fall back on linearising by hand. */
	if (ret == -ENOTSUPP) {
		len = map->format.reg_bytes + map->format.pad_bytes + val_len;
		buf = kzalloc(len, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;

		memcpy(buf, map->work_buf, map->format.reg_bytes);
		memcpy(buf + map->format.reg_bytes + map->format.pad_bytes,
		       val, val_len);
		ret = map->bus->write(map->bus_context, buf, len);

		kfree(buf);
	}

	return ret;
}

static inline bool regmap_can_raw_write(struct regmap *map)
{
	return map->bus && map->bus->write && map->format.format_val &&
		map->format.format_reg;
}

static inline int regmap_raw_write(struct regmap *map, unsigned int reg,
		     const void *val, size_t val_len)
{
	int ret;

	DRM_DEBUG("reg=0x%02x, val_len=%zu\n", reg, val_len);

	if (!regmap_can_raw_write(map))
		return -EINVAL;
	if (val_len % map->format.val_bytes)
		return -EINVAL;
	//if (map->max_raw_write && map->max_raw_write > val_len)
	//	return -E2BIG;

	//map->lock(map->lock_arg);

	ret = _regmap_raw_write(map, reg, val, val_len);

	//map->unlock(map->lock_arg);

	return ret;
}

static inline int regmap_raw_read(struct regmap *map, unsigned int reg,
				  void *val, size_t val_len)
{
	DRM_DEBUG("reg=0x%02x, val_len=%zu\n", reg, val_len);

	return 0;
}

static inline int _regmap_bus_read(void *context, unsigned int reg,
			    unsigned int *val)
{
	return -EINVAL;
}

static inline void regmap_format_8(void *buf, unsigned int val, unsigned int shift)
{
	u8 *b = buf;

	b[0] = val << shift;
}

static inline void regmap_parse_inplace_noop(void *buf)
{
}

static inline unsigned int regmap_parse_8(const void *buf)
{
	const u8 *b = buf;

	return b[0];
}

static inline int _regmap_bus_formatted_write(void *context, unsigned int reg,
				       unsigned int val)
{
	int ret;
	struct regmap *map = context;

	map->format.format_write(map, reg, val);

	ret = map->bus->write(map->bus_context, map->work_buf,
			      map->format.buf_size);

	return ret;
}

static inline int _regmap_bus_raw_write(void *context, unsigned int reg,
				 unsigned int val)
{
	struct regmap *map = context;

	map->format.format_val(map->work_buf + map->format.reg_bytes
			       + map->format.pad_bytes, val, 0);
	return _regmap_raw_write(map, reg,
				 map->work_buf +
				 map->format.reg_bytes +
				 map->format.pad_bytes,
				 map->format.val_bytes);
}

static inline struct regmap *__regmap_init(struct device *dev,
			     const struct regmap_bus *bus,
			     void *bus_context,
			     const struct regmap_config *config)
//			     struct lock_class_key *lock_key,
//			     const char *lock_name)
{
	struct regmap *map;
	int ret = -EINVAL;
	enum regmap_endian reg_endian, val_endian;
	//int i, j;

	if (!config)
		goto err;

	map = kzalloc(sizeof(*map), GFP_KERNEL);
	if (map == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	map->format.reg_bytes = DIV_ROUND_UP(config->reg_bits, 8);
	map->format.pad_bytes = config->pad_bits / 8;
	map->format.val_bytes = DIV_ROUND_UP(config->val_bits, 8);
	map->format.buf_size = DIV_ROUND_UP(config->reg_bits +
			config->val_bits + config->pad_bits, 8);
	map->reg_shift = config->pad_bits % 8;
	if (config->reg_stride)
		map->reg_stride = config->reg_stride;
	else
		map->reg_stride = 1;
	if (is_power_of_2(map->reg_stride))
		map->reg_stride_order = ilog2(map->reg_stride);
	else
		map->reg_stride_order = -1;
	map->use_single_read = config->use_single_rw || !bus || !bus->read;
	map->use_single_write = config->use_single_rw || !bus || !bus->write;
	map->can_multi_write = config->can_multi_write && bus && bus->write;
	if (bus) {
		map->max_raw_read = bus->max_raw_read;
		map->max_raw_write = bus->max_raw_write;
	}
	map->dev = dev;
	map->bus = bus;
	map->bus_context = bus_context;
//	map->max_register = config->max_register;
	//map->wr_table = config->wr_table;
	//map->rd_table = config->rd_table;
	//map->volatile_table = config->volatile_table;
	//map->precious_table = config->precious_table;
	//map->writeable_reg = config->writeable_reg;
	//map->readable_reg = config->readable_reg;
	//map->volatile_reg = config->volatile_reg;
	//map->precious_reg = config->precious_reg;
	map->cache_type = config->cache_type;
	//map->name = config->name;

	//spin_lock_init(&map->async_lock);
	//INIT_LIST_HEAD(&map->async_list);
	//INIT_LIST_HEAD(&map->async_free);
	//init_waitqueue_head(&map->async_waitq);

	//if (config->read_flag_mask || config->write_flag_mask) {
	//	map->read_flag_mask = config->read_flag_mask;
	//	map->write_flag_mask = config->write_flag_mask;
	//} else if (bus) {
	//	map->read_flag_mask = bus->read_flag_mask;
	//}

//	if (!bus) {
//		map->reg_read  = config->reg_read;
//		map->reg_write = config->reg_write;
//
//		map->defer_caching = false;
//		goto skip_format_initialization;
//	} else
//	if (!bus->read || !bus->write) {
//		map->reg_read = _regmap_bus_reg_read;
//		map->reg_write = _regmap_bus_reg_write;
//
//		map->defer_caching = false;
//		goto skip_format_initialization;
//	} else {
		map->reg_read  = _regmap_bus_read;
//		map->reg_update_bits = bus->reg_update_bits;
//	}

	reg_endian = regmap_get_reg_endian(bus, config);
	val_endian = regmap_get_val_endian(dev, bus, config);

	switch (config->reg_bits + map->reg_shift) {
#if 0
	case 2:
		switch (config->val_bits) {
		case 6:
			map->format.format_write = regmap_format_2_6_write;
			break;
		default:
			goto err_map;
		}
		break;

	case 4:
		switch (config->val_bits) {
		case 12:
			map->format.format_write = regmap_format_4_12_write;
			break;
		default:
			goto err_map;
		}
		break;

	case 7:
		switch (config->val_bits) {
		case 9:
			map->format.format_write = regmap_format_7_9_write;
			break;
		default:
			goto err_map;
		}
		break;

	case 10:
		switch (config->val_bits) {
		case 14:
			map->format.format_write = regmap_format_10_14_write;
			break;
		default:
			goto err_map;
		}
		break;
#endif
	case 8:
		map->format.format_reg = regmap_format_8;
		break;
#if 0
	case 16:
		switch (reg_endian) {
		case REGMAP_ENDIAN_BIG:
			map->format.format_reg = regmap_format_16_be;
			break;
		case REGMAP_ENDIAN_LITTLE:
			map->format.format_reg = regmap_format_16_le;
			break;
		case REGMAP_ENDIAN_NATIVE:
			map->format.format_reg = regmap_format_16_native;
			break;
		default:
			goto err_map;
		}
		break;

	case 24:
		if (reg_endian != REGMAP_ENDIAN_BIG)
			goto err_map;
		map->format.format_reg = regmap_format_24;
		break;

	case 32:
		switch (reg_endian) {
		case REGMAP_ENDIAN_BIG:
			map->format.format_reg = regmap_format_32_be;
			break;
		case REGMAP_ENDIAN_LITTLE:
			map->format.format_reg = regmap_format_32_le;
			break;
		case REGMAP_ENDIAN_NATIVE:
			map->format.format_reg = regmap_format_32_native;
			break;
		default:
			goto err_map;
		}
		break;
#endif
	default:
		goto err_map;
	}

	if (val_endian == REGMAP_ENDIAN_NATIVE)
		map->format.parse_inplace = regmap_parse_inplace_noop;

	switch (config->val_bits) {
	case 8:
		map->format.format_val = regmap_format_8;
		map->format.parse_val = regmap_parse_8;
		map->format.parse_inplace = regmap_parse_inplace_noop;
		break;
#if 0
	case 16:
		switch (val_endian) {
		case REGMAP_ENDIAN_BIG:
			map->format.format_val = regmap_format_16_be;
			map->format.parse_val = regmap_parse_16_be;
			map->format.parse_inplace = regmap_parse_16_be_inplace;
			break;
		case REGMAP_ENDIAN_LITTLE:
			map->format.format_val = regmap_format_16_le;
			map->format.parse_val = regmap_parse_16_le;
			map->format.parse_inplace = regmap_parse_16_le_inplace;
			break;
		case REGMAP_ENDIAN_NATIVE:
			map->format.format_val = regmap_format_16_native;
			map->format.parse_val = regmap_parse_16_native;
			break;
		default:
			goto err_map;
		}
		break;
	case 24:
		if (val_endian != REGMAP_ENDIAN_BIG)
			goto err_map;
		map->format.format_val = regmap_format_24;
		map->format.parse_val = regmap_parse_24;
		break;
	case 32:
		switch (val_endian) {
		case REGMAP_ENDIAN_BIG:
			map->format.format_val = regmap_format_32_be;
			map->format.parse_val = regmap_parse_32_be;
			map->format.parse_inplace = regmap_parse_32_be_inplace;
			break;
		case REGMAP_ENDIAN_LITTLE:
			map->format.format_val = regmap_format_32_le;
			map->format.parse_val = regmap_parse_32_le;
			map->format.parse_inplace = regmap_parse_32_le_inplace;
			break;
		case REGMAP_ENDIAN_NATIVE:
			map->format.format_val = regmap_format_32_native;
			map->format.parse_val = regmap_parse_32_native;
			break;
		default:
			goto err_map;
		}
		break;
#endif
	}

	if (map->format.format_write) {
		if ((reg_endian != REGMAP_ENDIAN_BIG) ||
		    (val_endian != REGMAP_ENDIAN_BIG))
			goto err_map;
		map->use_single_write = true;
	}

	if (!map->format.format_write &&
	    !(map->format.format_reg && map->format.format_val))
		goto err_map;

	map->work_buf = kzalloc(map->format.buf_size, GFP_KERNEL);
	if (map->work_buf == NULL) {
		ret = -ENOMEM;
		goto err_map;
	}

	if (map->format.format_write) {
//		map->defer_caching = false;
		map->reg_write = _regmap_bus_formatted_write;
	} else if (map->format.format_val) {
//		map->defer_caching = true;
		map->reg_write = _regmap_bus_raw_write;
	}

//skip_format_initialization:

//	if (dev) {
//		ret = regmap_attach_dev(dev, map, config);
//		if (ret != 0)
//			goto err_regcache;
//	}

	return map;

err_map:
	kfree(map);
err:
	return ERR_PTR(ret);
}

static inline struct regmap *devm_regmap_init(struct device *dev,
				  const struct regmap_bus *bus,
				  void *bus_context,
				  const struct regmap_config *config)
{

	return __regmap_init(dev, bus, bus_context, config);
}

#endif
