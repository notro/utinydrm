#ifndef __LINUX_REGULATOR_CONSUMER_H_
#define __LINUX_REGULATOR_CONSUMER_H_

static inline struct regulator *
devm_regulator_get_optional(struct device *dev, const char *id)
{
	return NULL;
}

static inline int regulator_enable(struct regulator *regulator)
{
	return 0;
}

static inline int regulator_disable(struct regulator *regulator)
{
	return 0;
}

#endif
