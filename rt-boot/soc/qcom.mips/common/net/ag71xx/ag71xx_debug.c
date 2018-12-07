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
#include <finsh/finsh.h>
#include <global/global.h>
#include <arch/cache.h>
#include <soc/net/ag71xx/regs.h>
#include <soc/net/ag71xx/ag71xx.h>
#include <soc/timer.h>

#ifdef AG71XX_DEBUG

struct ag71xx_debug ag_debug;

static void mdio(uint8_t argc, char **argv) 
{
	if (argc < 3) 
	{
		goto mdio_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		rt_uint32_t bus_id;
		if (!rt_strcmp(operator, "dump"))
		{
			if (argc < 3)
			{
				goto mdio_too_few_arg;
			}
			bus_id = rt_atol(argv[2]);			
			if(bus_id > 1)
			{
				goto mdio_bus_id_error;
			}
			if(!ag_debug.debug_mdio[bus_id])
			{
				goto mdio_not_exist;
			}
			ag_debug.debug_mdio[bus_id]->mdio_dump(ag_debug.debug_mdio[bus_id]);
		}
		else if (!rt_strcmp(operator, "write"))
		{
			rt_uint32_t addr;
			rt_uint32_t reg;
			rt_uint16_t value;
			
			if (argc < 6)
			{
				goto mdio_too_few_arg;
			}
			bus_id = rt_atol(argv[2]);
			if(bus_id > 1)
			{
				goto mdio_bus_id_error;
			}
			if(!ag_debug.debug_mdio[bus_id])
			{
				goto mdio_not_exist;
			}
			addr = rt_atol(argv[3]);
			reg = rt_atol(argv[4]);
			value = rt_atol(argv[5]);
			
			ag_debug.debug_mdio[bus_id]->mdio_write(ag_debug.debug_mdio[bus_id],addr,reg,value);
		}
		else if (!rt_strcmp(operator, "read"))
		{
			rt_uint32_t addr;
			rt_uint32_t reg;
			rt_uint16_t ret;
			
			if (argc < 5)
			{
				goto mdio_too_few_arg;
			}
			bus_id = rt_atol(argv[2]);
			if(bus_id > 1)
			{
				goto mdio_bus_id_error;
			}
			if(!ag_debug.debug_mdio[bus_id])
			{
				goto mdio_not_exist;
			}
			addr = rt_atol(argv[3]);
			reg = rt_atol(argv[4]);
			
			ret = ag_debug.debug_mdio[bus_id]->mdio_read(ag_debug.debug_mdio[bus_id],addr,reg);
		
			rt_kprintf("Read return 0x%04x\n",ret);
		}
		else
		{
			goto mdio_valid_cmd;
		}
	}
	
	return;

mdio_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto mdio_reprint;
mdio_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto mdio_reprint;
mdio_bus_id_error:
	rt_kprintf("Bus id error!\n");
	goto mdio_reprint;
mdio_not_exist:	
	rt_kprintf("Debug mdio is not exist!\n");
	goto mdio_reprint;
mdio_reprint:
	rt_kprintf("Try mdio dump <bus_id>\n");
	rt_kprintf("Try mdio read <bus_id> <addr> <reg>\n");
	rt_kprintf("Try mdio write <bus_id> <addr> <reg> <value>\n");
}

MSH_CMD_EXPORT(mdio, MDIO bus debug operation.);

static void phy_mdio(uint8_t argc, char **argv) 
{
	if (argc < 4) 
	{
		goto phy_mdio_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		
		if (!rt_strcmp(operator, "write"))
		{
			rt_uint32_t addr;
			rt_uint32_t reg;
			rt_uint16_t value;
			
			if (argc < 5)
			{
				goto phy_mdio_too_few_arg;
			}				
			if(!ag_debug.debug_phy_mdio)
			{
				goto phy_mdio_not_exist;
			}
			addr = rt_atol(argv[2]);
			reg = rt_atol(argv[3]);
			value = rt_atol(argv[4]);
			
			ag_debug.debug_phy_mdio->phy_write(ag_debug.debug_phy_mdio->am_bus,addr,reg,value);
		}
		else if (!rt_strcmp(operator, "read"))
		{
			rt_uint32_t addr;
			rt_uint32_t reg;
			rt_uint16_t ret;
			
			if (argc < 4)
			{
				goto phy_mdio_too_few_arg;
			}				
			if(!ag_debug.debug_phy_mdio)
			{
				goto phy_mdio_not_exist;
			}
			addr = rt_atol(argv[2]);
			reg = rt_atol(argv[3]);
			
			ret = ag_debug.debug_phy_mdio->phy_read(ag_debug.debug_phy_mdio->am_bus,addr,reg);
		
			rt_kprintf("Read return 0x%04x\n",ret);
		}
		else
		{
			goto phy_mdio_valid_cmd;
		}
	}
	
	return;

phy_mdio_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto phy_mdio_reprint;	
phy_mdio_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto phy_mdio_reprint;
phy_mdio_not_exist:	
	rt_kprintf("Debug phy_mdio is not exist!\n");
	goto phy_mdio_reprint;
phy_mdio_reprint:
	rt_kprintf("Try phy_mdio dump\n");
	rt_kprintf("Try phy_mdio read <addr> <reg>\n");
	rt_kprintf("Try phy_mdio write <addr> <reg> <value>\n");
}

MSH_CMD_EXPORT(phy_mdio, PHY-MDIO bus debug operation.);

static void switcher(uint8_t argc, char **argv) 
{
	if (argc < 3) 
	{
		goto switcher_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		
		if (!rt_strcmp(operator, "write"))
		{
			rt_uint32_t reg;
			rt_uint32_t value;
			
			if (argc < 4)
			{
				goto switcher_too_few_arg;
			}				
			if(!ag_debug.debug_switcher)
			{
				goto switcher_not_exist;
			}
			reg = rt_atol(argv[2]);
			value = rt_atol(argv[3]);

			ag_debug.debug_switcher->switch_reg_write(ag_debug.debug_switcher->am_bus,reg,value);
		}
		else if (!rt_strcmp(operator, "read"))
		{
			rt_uint32_t reg;
			rt_uint32_t ret;
			
			if (argc < 3)
			{
				goto switcher_too_few_arg;
			}
			if(!ag_debug.debug_switcher)
			{
				goto switcher_not_exist;
			}
			reg = rt_atol(argv[2]);
			
			ret = ag_debug.debug_switcher->switch_reg_read(ag_debug.debug_switcher->am_bus,reg);
		
			rt_kprintf("Read return 0x%08x\n",ret);
		}
		else
		{
			goto switcher_valid_cmd;
		}
	}
	
	return;

switcher_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto switcher_reprint;
switcher_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto switcher_reprint;
switcher_not_exist:	
	rt_kprintf("Debug switcher is not exist!\n");
	goto switcher_reprint;
switcher_reprint:
	rt_kprintf("Try switcher read <switcher_id> <reg>\n");
	rt_kprintf("Try switcher write <switcher_id> <reg> <value>\n");
}

MSH_CMD_EXPORT(switcher, Switcher debug operation.);

static void gmac(uint8_t argc, char **argv) 
{
	if (argc < 3) 
	{
		goto gmac_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		rt_uint32_t gmac_id;
		if (!rt_strcmp(operator, "dump"))
		{
			if (argc < 3)
			{
				goto gmac_too_few_arg;
			}
			gmac_id = rt_atol(argv[2]);
			if(gmac_id > 1)
			{
				goto gmac_id_error;
			}
			if(!ag_debug.debug_gmac[gmac_id])
			{
				goto gmac_not_exist;
			}
			
			ag_debug.debug_gmac[gmac_id]->gmac_dump(ag_debug.debug_gmac[gmac_id]);
		}
		else
		{
			goto gmac_valid_cmd;
		}
	}
	
	return;
	
gmac_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto gmac_reprint;
gmac_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto gmac_reprint;
gmac_id_error:
	rt_kprintf("Gmac id error!\n");
	goto gmac_reprint;
gmac_not_exist:	
	rt_kprintf("Debug gmac is not exist!\n");
	goto gmac_reprint;
gmac_reprint:
	rt_kprintf("Try gmac dump <gmac_id>\n");
}

MSH_CMD_EXPORT(gmac, Gmac debug operation.);

static void eth(uint8_t argc, char **argv) 
{
	if (argc < 3) 
	{
		goto eth_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		rt_uint32_t eth_id;
		if (!rt_strcmp(operator, "send"))
		{
			rt_uint8_t buffer[50];
			if (argc < 3)
			{
				goto eth_too_few_arg;
			}
			eth_id = rt_atol(argv[2]);
			if(eth_id > 1)
			{
				goto eth_id_error;
			}
			if(!ag_debug.debug_eth[eth_id])
			{
				goto eth_not_exist;
			}
			
			ag_debug.debug_eth[eth_id]->eth_send(ag_debug.debug_eth[eth_id],buffer,sizeof(buffer));
		}
		else
		{
			goto eth_valid_cmd;
		}
	}
	
	return;
	
eth_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto eth_reprint;
eth_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto eth_reprint;
eth_id_error:
	rt_kprintf("Eth id error!\n");
	goto eth_reprint;
eth_not_exist:	
	rt_kprintf("Debug eth is not exist!\n");
	goto eth_reprint;
eth_reprint:
	rt_kprintf("Try eth dump <gmac_id>\n");
}

MSH_CMD_EXPORT(eth, Eth debug operation.);

void ag71xx_debug_init(struct ag71xx_debug *ag_dbg)
{
	rt_memcpy(&ag_debug,ag_dbg,sizeof(struct ag71xx_debug));
}

#endif