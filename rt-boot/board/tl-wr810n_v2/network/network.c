/*
 * Qualcomm/Atheros High-Speed UART driver
 *
 * Copyright (C) 2015 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2014 Mantas Pucka <mantas@8devices.com>
 * Copyright (C) 2008-2010 Atheros Communications Inc.
 *
 * Values for UART_SCALE and UART_STEP:
 * https://www.mail-archive.com/openwrt-devel@lists.openwrt.org/msg22371.html
 *
 * Partially based on:
 * Linux/drivers/tty/serial/qca953x_uart.c
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <common/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_reset.h>

#include <soc/net/ag71xx/regs.h>
#include <soc/net/ag71xx/ag71xx.h>
#include <soc/timer.h>

static struct ag71xx_mdio am;
static struct ag71xx_phy ag_phy;
static struct ag71xx_switch ag_sw;
static struct ag71xx_gmac ag_gmac;
static struct ag71xx_eth ag_eth;
static struct ar7xxx_eth_fifos ag_fifos;
static struct ag71xx_platform_data ag_pdata;

int board_network_init(void)
{
	struct ag71xx_debug ag_dbg;
	
	ag71xx_mdio_struct_init(&am);
	ar7240sw_phy_struct_init(&ag_phy,&ag_sw);
	ag71xx_gmac_struct_init(&ag_gmac);
	ag71xx_eth_struct_init(&ag_eth);
	
	am.mdio_name = "ag7_mdio1";
	am.pdata = &ag_pdata;
	
	ag_phy.phy_name = "ag_phy0";
	ag_phy.am_bus = &am;
	ag_phy.pdata = &ag_pdata;
	
	ag_sw.switch_name = "ag_switch0";
	ag_sw.am_bus = &am;
	ag_sw.pdata = &ag_pdata;
	
	ag_gmac.gmac_name = "ag_gmac1";
	ag_gmac.pdata = &ag_pdata;
	
	ag_eth.eth_name = "ag_eth1";
	ag_eth.mdio = &am;
	ag_eth.gmac = &ag_gmac;
	ag_eth.phy = &ag_phy;
	ag_eth.switcher = &ag_sw;
	ag_eth.eth_fifos = &ag_fifos;
	ag_eth.pdata = &ag_pdata;
	
	ag_pdata.mac_base = 0x1A000000;
	ag_pdata.cfg_base = QCA_GMAC_BASE_REG;
	ag_pdata.reset_bit = QCA_RST_RESET_ETH_SWITCH_RST_MASK | 
		QCA_RST_RESET_GE0_MAC_RST_MASK | 
		QCA_RST_RESET_ETH_SWITCH_ARST_MASK | 
		QCA_RST_RESET_GE1_MAC_RST_MASK;
	ag_pdata.reset_addr = QCA_RST_RESET_REG;
	ag_pdata.int_number = 5;
	ag_pdata.is_ar934x = 1;
	ag_pdata.switch_only = 1;
	ag_pdata.max_frame_len = 1536;
	ag_pdata.builtin_switch = 1;
	ag_pdata.mdio_clock = 5000000;
	ag_pdata.ref_clock = 25000000;
	ag_pdata.mac_addr[0] = 22;
	ag_pdata.mac_addr[1] = 22;
	ag_pdata.mac_addr[2] = 22;
	ag_pdata.mac_addr[3] = 22;
	ag_pdata.mac_addr[4] = 22;
	ag_pdata.mac_addr[5] = 22;
	
	rt_memset(&ag_dbg,0,sizeof(struct ag71xx_debug));
	
	ag_dbg.debug_mdio[1] = &am;
	ag_dbg.debug_phy_mdio = &ag_phy;
	ag_dbg.debug_switcher = &ag_sw;
	ag_dbg.debug_gmac[1] = &ag_gmac;
	ag_dbg.debug_eth[1] = &ag_eth;
	
	ag71xx_debug_init(&ag_dbg);

	ag_gmac.gmac_init(&ag_gmac);
	am.mdio_init(&am);
	
	ag_phy.phy_init(ag_phy.am_bus);
	
	ag_eth.eth_init(&ag_eth);
	ag_eth.eth_start(&ag_eth);
	
	return 0;
}

void board_eth_recv_register_event(struct rt_event *event,rt_uint32_t id)
{
	ag_eth.eth_regis_recv_event(&ag_eth, event,id);
}

void board_eth_send_register_event(struct rt_event *event,rt_uint32_t id)
{
	ag_eth.eth_regis_send_event(&ag_eth, event,id);
}

void board_eth_recv(rt_uint8_t *ip_buffer,rt_uint32_t *ip_length)
{
	rt_uint8_t *buffer;
	rt_int32_t length;
	
	length = ag_eth.eth_recv(&ag_eth, 0, &buffer);
	if(length == -1)
		*ip_length = 0;
	else
	{
		rt_memcpy(ip_buffer, buffer, length);
		*ip_length = length;
	
		ag_eth.eth_free_pkt(&ag_eth, buffer, length);
	}
}

rt_int32_t board_eth_send(rt_uint8_t *ip_buffer,rt_uint32_t ip_length)
{
	return ag_eth.eth_send(&ag_eth, ip_buffer, ip_length);
}
