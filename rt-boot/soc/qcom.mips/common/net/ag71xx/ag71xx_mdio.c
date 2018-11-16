/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <common/global.h>
#include <soc/net/ag71xx/regs.h>
#include <soc/net/ag71xx/ag71xx.h>
#include <soc/timer.h>

#define AG71XX_MDIO_RETRY	1000
#define AG71XX_MDIO_DELAY	5

static inline void ag71xx_mdio_wr(struct ag71xx_mdio *am, rt_uint32_t reg,
				  rt_uint32_t value)
{
	rt_uint32_t r;
	rt_uint32_t mdio_base = am->pdata->mac_base;

	r = mdio_base + reg;
	ag71xx_writel(value, r);

	/* flush write */
	(void) ag71xx_readl(r);
}

static inline rt_uint32_t ag71xx_mdio_rr(struct ag71xx_mdio *am, rt_uint32_t reg)
{
	rt_uint32_t mdio_base = am->pdata->mac_base;
	return ag71xx_readl(mdio_base + reg);
}

static void ag71xx_mdio_dump_regs(struct ag71xx_mdio *am)
{
	rt_kprintf("%s: mii_cfg=%08x, mii_cmd=%08x, mii_addr=%08x\n",
				am->mdio_name,
				ag71xx_mdio_rr(am, AG71XX_REG_MII_CFG),
				ag71xx_mdio_rr(am, AG71XX_REG_MII_CMD),
				ag71xx_mdio_rr(am, AG71XX_REG_MII_ADDR));
	rt_kprintf("%s: mii_ctrl=%08x, mii_status=%08x, mii_ind=%08x\n",
				am->mdio_name,
				ag71xx_mdio_rr(am, AG71XX_REG_MII_CTRL),
				ag71xx_mdio_rr(am, AG71XX_REG_MII_STATUS),
				ag71xx_mdio_rr(am, AG71XX_REG_MII_IND));
}

static rt_int32_t ag71xx_mdio_wait_busy(struct ag71xx_mdio *am)
{
	rt_int32_t i;

	for (i = 0; i < AG71XX_MDIO_RETRY; i++) {
		rt_uint32_t busy;

		udelay(AG71XX_MDIO_DELAY);

		busy = ag71xx_mdio_rr(am, AG71XX_REG_MII_IND);
		if (!busy)
			return 0;

		udelay(AG71XX_MDIO_DELAY);
	}

	ag71xx_error_printf("%s: MDIO operation timed out\n", am->mdio_name);

	return -1;
}

static rt_uint16_t ag71xx_mdio_read(struct ag71xx_mdio *am, rt_uint32_t addr, rt_uint32_t reg)
{
	rt_uint32_t err;
	rt_uint16_t ret;

	err = ag71xx_mdio_wait_busy(am);
	if (err)
		return 0xffff;

	ag71xx_mdio_wr(am, AG71XX_REG_MII_CMD, MII_CMD_WRITE);
	ag71xx_mdio_wr(am, AG71XX_REG_MII_ADDR,
			((addr & 0xff) << MII_ADDR_SHIFT) | (reg & 0xff));
	ag71xx_mdio_wr(am, AG71XX_REG_MII_CMD, MII_CMD_READ);

	err = ag71xx_mdio_wait_busy(am);
	if (err)
		return 0xffff;

	ret = ag71xx_mdio_rr(am, AG71XX_REG_MII_STATUS) & 0xffff;
	ag71xx_mdio_wr(am, AG71XX_REG_MII_CMD, MII_CMD_WRITE);

	ag71xx_debug_printf("mii_read: addr=%04x, reg=%04x, value=%04x\n", addr, reg, ret);

	return ret;
}

static void ag71xx_mdio_write(struct ag71xx_mdio *am, rt_uint32_t addr, rt_uint32_t reg, rt_uint16_t val)
{
	ag71xx_debug_printf("mii_write: addr=%04x, reg=%04x, value=%04x\n", addr, reg, val);

	ag71xx_mdio_wait_busy(am);
	
	ag71xx_mdio_wr(am, AG71XX_REG_MII_ADDR,
			((addr & 0xff) << MII_ADDR_SHIFT) | (reg & 0xff));
	ag71xx_mdio_wr(am, AG71XX_REG_MII_CTRL, val);

	ag71xx_mdio_wait_busy(am);
}

static const rt_uint32_t ar71xx_mdio_div_table[] = {
	4, 4, 6, 8, 10, 14, 20, 28,
};

static const rt_uint32_t ar7240_mdio_div_table[] = {
	2, 2, 4, 6, 8, 12, 18, 26, 32, 40, 48, 56, 62, 70, 78, 96,
};

static const rt_uint32_t ar933x_mdio_div_table[] = {
	4, 4, 6, 8, 10, 14, 20, 28, 34, 42, 50, 58, 66, 74, 82, 98,
};

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

static rt_int32_t ag71xx_mdio_get_divider(struct ag71xx_mdio *am, rt_uint32_t *div)
{
	rt_uint64_t ref_clock, mdio_clock;
	const rt_uint32_t *table;
	rt_int32_t ndivs;
	rt_int32_t i;

	ref_clock = am->pdata->ref_clock;
	mdio_clock = am->pdata->mdio_clock;

	if (!ref_clock || !mdio_clock)
		return -1;

	if (am->pdata->is_ar9330 || am->pdata->is_ar934x) {
		table = ar933x_mdio_div_table;
		ndivs = ARRAY_SIZE(ar933x_mdio_div_table);
	} else if (am->pdata->is_ar7240) {
		table = ar7240_mdio_div_table;
		ndivs = ARRAY_SIZE(ar7240_mdio_div_table);
	} else {
		table = ar71xx_mdio_div_table;
		ndivs = ARRAY_SIZE(ar71xx_mdio_div_table);
	}

	for (i = 0; i < ndivs; i++) {
		rt_uint64_t t;

		t = ref_clock / table[i];
		if (t <= mdio_clock) {
			*div = i;
			return 0;
		}
	}

	ag71xx_error_printf("%s:no divider found for %lu/%lu\n",
		am->mdio_name, ref_clock, mdio_clock);
	
	return -1;
}

static rt_int32_t ag71xx_mdio_reset(struct ag71xx_mdio *am)
{
	rt_uint32_t t;
	rt_int32_t err;

	err = ag71xx_mdio_get_divider(am, &t);
	if (err) {
		/* fallback */
		if (am->pdata->is_ar7240)
			t = MII_CFG_CLK_DIV_6;
		else if (am->pdata->builtin_switch && !am->pdata->is_ar934x)
			t = MII_CFG_CLK_DIV_10;
		else if (!am->pdata->builtin_switch && am->pdata->is_ar934x)
			t = MII_CFG_CLK_DIV_58;
		else
			t = MII_CFG_CLK_DIV_28;
	}

	ag71xx_mdio_wr(am, AG71XX_REG_MII_CFG, t | MII_CFG_RESET);
	udelay(100);

	ag71xx_mdio_wr(am, AG71XX_REG_MII_CFG, t);
	udelay(100);

	return 0;
}

static rt_int32_t ag71xx_mdio_init(struct ag71xx_mdio *am)
{
	//ag71xx_mdio_wr(am, AG71XX_REG_MAC_CFG1, 0);

	ag71xx_mdio_reset(am);
	
	return 0;
}

void ag71xx_mdio_struct_init(struct ag71xx_mdio *am)
{
	am->mdio_init = ag71xx_mdio_init;
	am->mdio_dump = ag71xx_mdio_dump_regs;
    am->mdio_read = ag71xx_mdio_read;
    am->mdio_write = ag71xx_mdio_write;
	am->mdio_reset = ag71xx_mdio_reset;
}
