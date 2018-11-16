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

#define MAC_CFG1_INIT	(MAC_CFG1_RXE | MAC_CFG1_TXE | \
			 MAC_CFG1_SRX | MAC_CFG1_STX)

#define FIFO_CFG0_INIT	(FIFO_CFG0_ALL << FIFO_CFG0_ENABLE_SHIFT)

#define FIFO_CFG4_INIT	(FIFO_CFG4_DE | FIFO_CFG4_DV | FIFO_CFG4_FC | \
			 FIFO_CFG4_CE | FIFO_CFG4_CR | FIFO_CFG4_LM | \
			 FIFO_CFG4_LO | FIFO_CFG4_OK | FIFO_CFG4_MC | \
			 FIFO_CFG4_BC | FIFO_CFG4_DR | FIFO_CFG4_LE | \
			 FIFO_CFG4_CF | FIFO_CFG4_PF | FIFO_CFG4_UO | \
			 FIFO_CFG4_VT)

#define FIFO_CFG5_INIT	(FIFO_CFG5_DE | FIFO_CFG5_DV | FIFO_CFG5_FC | \
			 FIFO_CFG5_CE | FIFO_CFG5_LO | FIFO_CFG5_OK | \
			 FIFO_CFG5_MC | FIFO_CFG5_BC | FIFO_CFG5_DR | \
			 FIFO_CFG5_CF | FIFO_CFG5_PF | FIFO_CFG5_VT | \
			 FIFO_CFG5_LE | FIFO_CFG5_FT | FIFO_CFG5_16 | \
			 FIFO_CFG5_17 | FIFO_CFG5_SF)

static inline void ag71xx_gmac_wr(struct ag71xx_gmac *ag_gmac, rt_uint32_t reg,
				  rt_uint32_t value)
{
	rt_uint32_t r;
	rt_uint32_t gmac_base = ag_gmac->pdata->mac_base;

	r = gmac_base + reg;
	ag71xx_writel(value, r);

	/* flush write */
	(void) ag71xx_readl(r);
}

static inline void ag71xx_gmac_sb(struct ag71xx_gmac *ag_gmac, rt_uint32_t reg,
				  rt_uint32_t mask)
{
	rt_uint32_t r;

	r = ag_gmac->pdata->mac_base + reg;
	ag71xx_writel(ag71xx_readl(r) | mask, r);
	/* flush write */
	(void)ag71xx_readl(r);
}

static inline rt_uint32_t ag71xx_gmac_rr(struct ag71xx_gmac *ag_gmac, rt_uint32_t reg)
{
	rt_uint32_t gmac_base = ag_gmac->pdata->mac_base;
	return ag71xx_readl(gmac_base + reg);
}

static void ag71xx_gmac_dump_regs(struct ag71xx_gmac *ag_gmac)
{
	rt_kprintf("%s: mac_cfg1=%08x, mac_cfg2=%08x, ipg=%08x, hdx=%08x, mfl=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_CFG1),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_CFG2),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_IPG),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_HDX),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_MFL));
	rt_kprintf("%s: mac_ifctl=%08x, mac_addr1=%08x, mac_addr2=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_IFCTL),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_ADDR1),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_MAC_ADDR2));
	rt_kprintf("%s: fifo_cfg0=%08x, fifo_cfg1=%08x, fifo_cfg2=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG0),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG1),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG2));
	rt_kprintf("%s: fifo_cfg3=%08x, fifo_cfg4=%08x, fifo_cfg5=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG3),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG4),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_FIFO_CFG5));
	rt_kprintf("%s: dma_tx_ctrl=%08x, dma_tx_desc=%08x, dma_tx_status=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_TX_CTRL),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_TX_DESC),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_TX_STATUS));
	rt_kprintf("%s: dma_rx_ctrl=%08x, dma_rx_desc=%08x, dma_rx_status=%08x\n",
		ag_gmac->gmac_name,
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_RX_CTRL),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_RX_DESC),
		ag71xx_gmac_rr(ag_gmac, AG71XX_REG_RX_STATUS));
}

static void ag71xx_gmac_stop(struct ag71xx_gmac *ag_gmac)
{
	/* disable all interrupts and stop the rx/tx engine */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_INT_ENABLE, 0);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_RX_CTRL, 0);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_TX_CTRL, 0);
}

static void ag71xx_gmac_setup(struct ag71xx_gmac *ag_gmac)
{
	struct ag71xx_platform_data *pdata = ag_gmac->pdata;
	rt_uint32_t init = MAC_CFG1_INIT;

	/* setup MAC configuration registers */
	if (pdata->use_flow_control)
		init |= MAC_CFG1_TFC | MAC_CFG1_RFC;
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_MAC_CFG1, init);

	ag71xx_gmac_sb(ag_gmac, AG71XX_REG_MAC_CFG2,
		  MAC_CFG2_IF_1000 | MAC_CFG2_PAD_CRC_EN | MAC_CFG2_LEN_CHECK | MAC_CFG2_FDX);

	/* setup max frame length to zero */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_MAC_MFL, 0x600);

	/* setup FIFO configuration registers */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG0, 0x001f1f00);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG1, 0x0010ffff);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG2, 0x02aa0155);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG3, 0x01f00140);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG4, 0x0003ffff);
	/*
	 * When enable the web failsafe mode in uboot,you can't drop the broadcast
	 * frames now,the PC first will tx a ARP request packet, it's a broadcast packet.
	 */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_FIFO_CFG5, 0x00066b82);
}

static void ag71xx_dma_reset(struct ag71xx_gmac *ag_gmac)
{
	rt_uint32_t val;
	int i;

	/* stop RX and TX */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_RX_CTRL, 0);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_TX_CTRL, 0);

	/*
	 * give the hardware some time to really stop all rx/tx activity
	 * clearing the descriptors too early causes random memory corruption
	 */
	mdelay(1);

	/* clear descriptor addresses */
	//ag71xx_gmac_wr(ag_gmac, AG71XX_REG_TX_DESC, ag_gmac->stop_desc_dma);
	//ag71xx_gmac_wr(ag_gmac, AG71XX_REG_RX_DESC, ag_gmac->stop_desc_dma);

	/* clear pending RX/TX interrupts */
	for (i = 0; i < 256; i++) {
		ag71xx_gmac_wr(ag_gmac, AG71XX_REG_RX_STATUS, RX_STATUS_PR);
		ag71xx_gmac_wr(ag_gmac, AG71XX_REG_TX_STATUS, TX_STATUS_PS);
	}

	/* clear pending errors */
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_RX_STATUS, RX_STATUS_BE | RX_STATUS_OF);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_TX_STATUS, TX_STATUS_BE | TX_STATUS_UR);

	val = ag71xx_gmac_rr(ag_gmac, AG71XX_REG_RX_STATUS);
	if (val)
		ag71xx_error_printf("%s: unable to clear DMA Rx status: %08x\n",
			 ag_gmac->gmac_name, val);

	val = ag71xx_gmac_rr(ag_gmac, AG71XX_REG_TX_STATUS);

	/* mask out reserved bits */
	val &= ~0xff000000;

	if (val)
		ag71xx_error_printf("%s: unable to clear DMA Tx status: %08x\n",
			 ag_gmac->gmac_name, val);
}

static void ag71xx_gmac_set_macaddr(struct ag71xx_gmac *ag_gmac, rt_uint8_t *mac)
{
	rt_uint32_t t;

	t = (((rt_uint32_t) mac[5]) << 24) | (((rt_uint32_t) mac[4]) << 16)
	  | (((rt_uint32_t) mac[3]) << 8) | ((rt_uint32_t) mac[2]);

	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_MAC_ADDR1, t);

	t = (((rt_uint32_t) mac[1]) << 24) | (((rt_uint32_t) mac[0]) << 16);
	ag71xx_gmac_wr(ag_gmac, AG71XX_REG_MAC_ADDR2, t);
}

#define AR71XX_RESET_GE0_PHY		BIT(8)
#define AR71XX_RESET_GE1_PHY		BIT(12)

static void ag71xx_gmac_init(struct ag71xx_gmac *ag_gmac)
{
	struct ag71xx_platform_data *pdata = ag_gmac->pdata;
	rt_uint32_t reset_mask = pdata->reset_bit;
    rt_uint32_t reset_addr = pdata->reset_addr;

	ag71xx_gmac_stop(ag_gmac);

	if (pdata->is_ar724x) {
		rt_uint32_t reset_phy = reset_mask;

		reset_phy &= AR71XX_RESET_GE0_PHY | AR71XX_RESET_GE1_PHY;
		reset_mask &= ~(AR71XX_RESET_GE0_PHY | AR71XX_RESET_GE1_PHY);

        ag71xx_writel((ag71xx_readl((reset_addr)) | (reset_phy)), (reset_addr));
		mdelay(50);
        ag71xx_writel((ag71xx_readl((reset_addr)) & ~(reset_phy)), (reset_addr));
		mdelay(200);
	}

	ag71xx_gmac_sb(ag_gmac, AG71XX_REG_MAC_CFG1, MAC_CFG1_SR);
	udelay(20);

    ag71xx_writel((ag71xx_readl((reset_addr)) | (reset_mask)), (reset_addr));
	mdelay(100);
    ag71xx_writel((ag71xx_readl((reset_addr)) & ~(reset_mask)), (reset_addr));
	mdelay(200);

	if(pdata->switch_only)
		ag71xx_writel(0x40,pdata->cfg_base);
	
	ag71xx_gmac_setup(ag_gmac);

	ag71xx_dma_reset(ag_gmac);
	
	ag71xx_gmac_set_macaddr(ag_gmac,ag_gmac->pdata->mac_addr);	
}

void ag71xx_gmac_struct_init(struct ag71xx_gmac *ag_gmac)
{
	ag_gmac->gmac_init = ag71xx_gmac_init;
	ag_gmac->gmac_dump = ag71xx_gmac_dump_regs;
}
