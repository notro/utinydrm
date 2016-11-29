#ifndef __LINUX_REGMAP_H
#define __LINUX_REGMAP_H

enum regmap_endian {
	/* Unspecified -> 0 -> Backwards compatible default */
	REGMAP_ENDIAN_DEFAULT = 0,
	REGMAP_ENDIAN_BIG,
	REGMAP_ENDIAN_LITTLE,
	REGMAP_ENDIAN_NATIVE,
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

	struct device *dev; /* Device we do I/O on */
	void *work_buf;     /* Scratch buffer used to format I/O */
	struct regmap_format format;  /* Buffer format */
	const struct regmap_bus *bus;
	void *bus_context;
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

	int (*reg_read)(void *context, unsigned int reg, unsigned int *val);
	int (*reg_write)(void *context, unsigned int reg, unsigned int val);
	int (*reg_update_bits)(void *context, unsigned int reg,
			       unsigned int mask, unsigned int val);

	bool defer_caching;

	unsigned long read_flag_mask;
	unsigned long write_flag_mask;

	/* number of bits to (left) shift the reg value when formatting*/
	int reg_shift;
	int reg_stride;
	int reg_stride_order;

	/* regcache specific members */
	const struct regcache_ops *cache_ops;
	enum regcache_type cache_type;

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

	/* if set, converts bulk read to single read */
	bool use_single_read;
	/* if set, converts bulk read to single read */
	bool use_single_write;
	/* if set, the device supports multi write mode */
	bool can_multi_write;

	/* if set, raw reads/writes are limited to this size */
	size_t max_raw_read;
	size_t max_raw_write;

	struct rb_root range_tree;
	void *selector_work_buf;	/* Scratch buffer used for selector */
#endif
};

static inline int regmap_raw_write(struct regmap *map, unsigned int reg,
		     const void *val, size_t val_len)
{
	return 0;
}

#endif
