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
#include <global/global.h>
#include <arch/cache.h>
#include <soc/net/ag71xx/regs.h>
#include <soc/net/ag71xx/ag71xx.h>
#include <soc/timer.h>

static inline void ag71xx_eth_wr(struct ag71xx_eth *ag_eth, rt_uint32_t reg,
				  rt_uint32_t value)
{
	rt_uint32_t r;
	rt_uint32_t gmac_base = ag_eth->pdata->mac_base;

	r = gmac_base + reg;
	ag71xx_writel(value, r);

	/* flush write */
	(void) ag71xx_readl(r);
}

static inline rt_uint32_t ag71xx_eth_rr(struct ag71xx_eth *ag_eth, rt_uint32_t reg)
{
	rt_uint32_t r;
	rt_uint32_t gmac_base = ag_eth->pdata->mac_base;

	r = gmac_base + reg;
	
	return ag71xx_readl(r);
}

static inline void ag71xx_eth_sb(struct ag71xx_eth *ag_eth, rt_uint32_t reg,
				  rt_uint32_t mask)
{
	rt_uint32_t r;

	r = ag_eth->pdata->mac_base + reg;
	ag71xx_writel(ag71xx_readl(r) | mask, r);
	/* flush write */
	(void)ag71xx_readl(r);
}

static inline void ag71xx_eth_cb(struct ag71xx_eth *ag_eth, rt_uint32_t reg,
				  rt_uint32_t mask)
{
	rt_uint32_t r;

	r = ag_eth->pdata->mac_base + reg;
	ag71xx_writel(ag71xx_readl(r)  & ~mask, r);
	/* flush write */
	(void)ag71xx_readl(r);
}

static inline void ag71xx_int_enable(struct ag71xx_eth *ag_eth, rt_uint32_t ints)
{
	ag71xx_eth_sb(ag_eth, AG71XX_REG_INT_ENABLE, ints);
}

static inline void ag71xx_int_disable(struct ag71xx_eth *ag_eth, rt_uint32_t ints)
{
	ag71xx_eth_cb(ag_eth, AG71XX_REG_INT_ENABLE, ints);
}

/*
 * DMA ring handlers
 */
static void ag71xx_dma_clean_tx(struct ag71xx_eth *ag_eth)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;
	struct ag71xx_dma_desc *curr, *next;
	rt_int32_t i;

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		curr = &eth_fifos->tx_mac_descrtable[i];
		next = &eth_fifos->tx_mac_descrtable[(i + 1) % CONFIG_TX_DESCR_NUM];

        curr->data_addr = PHYSADDR(&eth_fifos->txbuffs[i * CONFIG_ETH_BUFSIZE]);
		curr->config = AG71XX_DMADESC_IS_EMPTY;
        curr->next_desc = PHYSADDR(next);
	}

	eth_fifos->tx_currdescnum = 0;

	/* Cache: Flush descriptors, don't care about buffers. */
	arch_dcache_flush((rt_uint32_t)(eth_fifos->tx_mac_descrtable), sizeof(struct ag71xx_dma_desc) * CONFIG_TX_DESCR_NUM);
}

static void ag71xx_dma_clean_rx(struct ag71xx_eth *ag_eth)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;
	struct ag71xx_dma_desc *curr, *next;
	rt_int32_t i;

	for (i = 0; i < CONFIG_RX_DESCR_NUM; i++) {
		curr = &eth_fifos->rx_mac_descrtable[i];
		next = &eth_fifos->rx_mac_descrtable[(i + 1) % CONFIG_RX_DESCR_NUM];

        curr->data_addr = PHYSADDR(&eth_fifos->rxbuffs[i * CONFIG_ETH_BUFSIZE]);
		curr->config = AG71XX_DMADESC_IS_EMPTY;
        curr->next_desc = PHYSADDR(next);
	}

	eth_fifos->rx_currdescnum = 0;

	/* Cache: Flush+Invalidate descriptors, Invalidate buffers. */
	arch_dcache_flush((rt_uint32_t)(eth_fifos->rx_mac_descrtable), sizeof(struct ag71xx_dma_desc) * CONFIG_RX_DESCR_NUM);
	arch_dcache_invalidate((rt_uint32_t)(eth_fifos->rx_mac_descrtable), sizeof(struct ag71xx_dma_desc) * CONFIG_RX_DESCR_NUM);
	arch_dcache_invalidate((rt_uint32_t)eth_fifos->rxbuffs, sizeof(rt_int8_t) * RX_TOTAL_BUFSIZE);
}

/*
 * Ethernet I/O
 */
static rt_int32_t ag71xx_eth_send(struct ag71xx_eth *ag_eth, void *packet, rt_int32_t length)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;
	struct ag71xx_dma_desc *curr;

	curr = &eth_fifos->tx_mac_descrtable[eth_fifos->tx_currdescnum];

	/* Cache: Invalidate descriptor. */
	arch_dcache_invalidate((rt_uint32_t)curr, sizeof(struct ag71xx_dma_desc));

	if (!(curr->config & AG71XX_DMADESC_IS_EMPTY)) {
		return -1;
	}

	/* Copy the packet into the data buffer. */
	rt_memcpy((void *)KSEG0ADDR(curr->data_addr), packet, length);
	curr->config = length & AG71XX_DMADESC_PKT_SIZE_MASK;

	/* Cache: Flush descriptor, Flush buffer. */
	arch_dcache_flush((rt_uint32_t)curr, sizeof(struct ag71xx_dma_desc));
	arch_dcache_flush((rt_uint32_t)KSEG0ADDR(curr->data_addr), length);

	/* Load the DMA descriptor and start TX DMA. */
	ag71xx_eth_wr(ag_eth, AG71XX_REG_TX_CTRL, TX_CTRL_TXE);

	/* Switch to next TX descriptor. */
	eth_fifos->tx_currdescnum = (eth_fifos->tx_currdescnum + 1) % CONFIG_TX_DESCR_NUM;

	return 0;
}

static void ag71xx_eth_regis_send_event(struct ag71xx_eth *ag_eth, struct rt_event *event,rt_uint32_t id)
{
	ag_eth->eth_send_event = event;
	ag_eth->eth_send_event_id = id;
}

static rt_int32_t ag71xx_eth_recv(struct ag71xx_eth *ag_eth, rt_int32_t flags, rt_uint8_t **packetp)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;
	struct ag71xx_dma_desc *curr;
	rt_uint32_t length;

	curr = &eth_fifos->rx_mac_descrtable[eth_fifos->rx_currdescnum];

	/* Cache: Invalidate descriptor. */
	arch_dcache_invalidate((rt_uint32_t)curr, sizeof(struct ag71xx_dma_desc));

	/* No packets received. */
	if (curr->config & AG71XX_DMADESC_IS_EMPTY)
		return -1;

	length = curr->config & AG71XX_DMADESC_PKT_SIZE_MASK;

	/* Cache: Invalidate buffer. */
	arch_dcache_invalidate((rt_uint32_t)KSEG0ADDR(curr->data_addr), length);

	/* Receive one packet and return length. */
	*packetp = (rt_uint8_t *)KSEG0ADDR(curr->data_addr);
	return length;
}

static rt_int32_t ag71xx_eth_free_pkt(struct ag71xx_eth *ag_eth, rt_uint8_t *packet,
				   rt_int32_t length)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;
	struct ag71xx_dma_desc *curr;

	curr = &eth_fifos->rx_mac_descrtable[eth_fifos->rx_currdescnum];

	curr->config = AG71XX_DMADESC_IS_EMPTY;

	/* Cache: Flush descriptor. */
	arch_dcache_flush((rt_uint32_t)curr, sizeof(struct ag71xx_dma_desc));

	/* Switch to next RX descriptor. */
	eth_fifos->rx_currdescnum = (eth_fifos->rx_currdescnum + 1) % CONFIG_RX_DESCR_NUM;
	
	/* Start RX DMA. */
	ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);

	return 0;
}

static void ag71xx_eth_regis_recv_event(struct ag71xx_eth *ag_eth, struct rt_event *event,rt_uint32_t id)
{
	ag_eth->eth_recv_event = event;
	ag_eth->eth_recv_event_id = id;
}

static rt_int32_t ag71xx_eth_start(struct ag71xx_eth *ag_eth)
{
	struct ar7xxx_eth_fifos *eth_fifos = ag_eth->eth_fifos;

	/* FIXME: Check if link up */

	/* Clear the DMA rings. */
	ag71xx_dma_clean_tx(ag_eth);
	ag71xx_dma_clean_rx(ag_eth);

	/* Load DMA descriptors. */
    ag71xx_eth_wr(ag_eth, AG71XX_REG_TX_DESC, PHYSADDR(&eth_fifos->tx_mac_descrtable[eth_fifos->tx_currdescnum]));
    ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_DESC, PHYSADDR(&eth_fifos->rx_mac_descrtable[eth_fifos->rx_currdescnum]));
	
	ag71xx_int_enable(ag_eth, AG71XX_INT_POLL);
	
	/* Start RX DMA. */
	ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);

	return 0;
}

static void ag71xx_eth_stop(struct ag71xx_eth *ag_eth)
{
	ag71xx_int_disable(ag_eth, AG71XX_INT_POLL);
	
	/* Stop the TX DMA. */
	ag71xx_eth_wr(ag_eth, AG71XX_REG_TX_CTRL, 0);

	/* Stop the RX DMA. */
	ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_CTRL, 0);
}

static void ag71xx_eth_irq_handler(int vector,void * para)
{
	struct ag71xx_eth *ag_eth = para;
	rt_uint32_t status;
	
	status = ag71xx_eth_rr(ag_eth, AG71XX_REG_INT_STATUS);
	
	ag71xx_debug_printf("%s:Int status: 0x%08x\n", ag_eth->eth_name,status);
	
	if (status & AG71XX_INT_ERR) 
	{
		if (status & AG71XX_INT_TX_BE) 
		{
			ag71xx_eth_wr(ag_eth, AG71XX_REG_TX_STATUS, TX_STATUS_BE);
			ag71xx_error_printf("%s:TX BUS error\n",ag_eth->eth_name);
		}
		if (status & AG71XX_INT_RX_BE) 
		{
			ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_STATUS, RX_STATUS_BE);
			ag71xx_error_printf("%s:RX BUS error\n",ag_eth->eth_name);
		}
	}
	
	if (status & AG71XX_INT_POLL) 
	{
		ag71xx_debug_printf("%s:Data ready\n", ag_eth->eth_name);
		
		if (status & AG71XX_INT_TX)
		{
			while (ag71xx_eth_rr(ag_eth, AG71XX_REG_TX_STATUS) & TX_STATUS_PS)
			{
				ag71xx_eth_wr(ag_eth, AG71XX_REG_TX_STATUS, TX_STATUS_PS);
			}
			
			if(ag_eth->eth_send_event)
			{
				rt_event_send(ag_eth->eth_send_event, (1 << ag_eth->eth_send_event_id));
			}
		}
		
		if (status & AG71XX_INT_RX)
		{
			if (ag71xx_eth_rr(ag_eth, AG71XX_REG_RX_STATUS) & RX_STATUS_OF)
			{
				ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_STATUS, RX_STATUS_OF);
			}
			
			while (ag71xx_eth_rr(ag_eth, AG71XX_REG_RX_STATUS) & RX_STATUS_PR)
			{
				ag71xx_eth_wr(ag_eth, AG71XX_REG_RX_STATUS, RX_STATUS_PR);
			}
			
			if(ag_eth->eth_recv_event)
			{
				rt_event_send(ag_eth->eth_recv_event, (1 << ag_eth->eth_recv_event_id));
			}
		}
	}
}

static rt_int32_t ag71xx_eth_init(struct ag71xx_eth *ag_eth)
{
	rt_uint8_t *buffer;
	rt_uint32_t size;
	
	size = sizeof(struct ag71xx_dma_desc) * CONFIG_TX_DESCR_NUM;
	if(rtboot_data.dcache_line_size)
		size += rtboot_data.dcache_line_size - 1;
	
	buffer = rt_malloc(size);
	if(!buffer)
	{
		rt_kprintf("Fail to alloc tx_dma_descs");
		return -1;
	}
	
	if(rtboot_data.dcache_line_size)
		buffer = (rt_uint8_t *) (((rt_uint32_t) buffer + rtboot_data.dcache_line_size - 1) &
				~(rtboot_data.dcache_line_size - 1));
	
	ag_eth->eth_fifos->tx_mac_descrtable = (struct ag71xx_dma_desc	*)buffer;
	
	buffer = rt_malloc(size);
	if(!buffer)
	{
		rt_kprintf("Fail to alloc rx_dma_descs");
		return -1;
	}
	
	if(rtboot_data.dcache_line_size)
		buffer = (rt_uint8_t *) (((rt_uint32_t) buffer + rtboot_data.dcache_line_size - 1) &
				~(rtboot_data.dcache_line_size - 1));
	
	ag_eth->eth_fifos->rx_mac_descrtable = (struct ag71xx_dma_desc	*)buffer;
	
	size = sizeof(rt_int8_t) * TX_TOTAL_BUFSIZE;
	if(rtboot_data.dcache_size)
		size += rtboot_data.dcache_size - 1;
	
	buffer = rt_malloc(size);
	if(!buffer)
	{
		rt_kprintf("Fail to alloc txbuffs");
		return -1;
	}
	
	if(rtboot_data.dcache_size)
		buffer = (rt_uint8_t *) (((rt_uint32_t) buffer + rtboot_data.dcache_size - 1) &
				~(rtboot_data.dcache_size - 1));
	
	ag_eth->eth_fifos->txbuffs = (rt_int8_t *)buffer;
	
	buffer = rt_malloc(size);
	if(!buffer)
	{
		rt_kprintf("Fail to alloc rxbuffs");
		return -1;
	}
	
	if(rtboot_data.dcache_size)
		buffer = (rt_uint8_t *) (((rt_uint32_t) buffer + rtboot_data.dcache_size - 1) &
				~(rtboot_data.dcache_size - 1));
	
	ag_eth->eth_fifos->rxbuffs = (rt_int8_t *)buffer;
	
	rt_hw_interrupt_install(ag_eth->pdata->int_number, ag71xx_eth_irq_handler,(void *)ag_eth, "ag_eth int handler");
	
    rt_hw_interrupt_mask(ag_eth->pdata->int_number);
	
	return 0;
}

void ag71xx_eth_struct_init(struct ag71xx_eth *ag_eth)
{
	ag_eth->eth_init = ag71xx_eth_init;
	ag_eth->eth_send = ag71xx_eth_send;
	ag_eth->eth_regis_send_event = ag71xx_eth_regis_send_event;
	ag_eth->eth_regis_recv_event = ag71xx_eth_regis_recv_event;
	ag_eth->eth_recv = ag71xx_eth_recv;
	ag_eth->eth_free_pkt = ag71xx_eth_free_pkt;
	ag_eth->eth_start = ag71xx_eth_start;
	ag_eth->eth_stop = ag71xx_eth_stop;
}