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
#include <soc/qca953x/qca953x_ls_uart.h>
#include <soc/qca953x/qca953x_irq.h>

void rt_hw_console_init(void)
{
	qca953x_lsuart_init();
}

void rt_hw_console_output(const char *str)
{
    qca953x_lsuart_puts(str);
}

struct rt_device soc_console_dev;

static rt_err_t rt_qca953x_lsuart_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_qca953x_lsuart_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_qca953x_lsuart_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	char *ptr = (char*) buffer;
	for(;size > 0;size--)
	{
		if(!qca953x_lsuart_tstc())
			break;
		*ptr++ = qca_soc_reg_read(QCA_LSUART_RBR_REG)& QCA_LSUART_RBR_RBR_MASK;
	}
	
	if(size > 0)
		qca_soc_reg_read_set(QCA_LSUART_IER_REG, QCA_LSUART_IER_ERBFI_MASK);
	
	return (rt_size_t)ptr - (rt_size_t)buffer;
}

static rt_size_t rt_qca953x_lsuart_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	char *ptr = (char*) buffer;
	for(;size > 0;size--)
	{
		qca953x_lsuart_putc(*ptr++);
	}
	return (rt_size_t)ptr - (rt_size_t)buffer;
}

static void rt_qca953x_lsuart_irq_handler(int vector,void * para)
{
	qca_soc_reg_read(QCA_LSUART_IIR_REG);
	if(qca_soc_reg_read(QCA_LSUART_LSR_REG) & QCA_LSUART_LSR_DR_MASK)
	{
		if (soc_console_dev.rx_indicate != RT_NULL)
    	{
			qca_soc_reg_read_clear(QCA_LSUART_IER_REG, QCA_LSUART_IER_ERBFI_MASK);
        	soc_console_dev.rx_indicate(&soc_console_dev, 1);
    	}
		else
		{
			while(1)
			{
				if(!qca953x_lsuart_tstc())
					break;
				if(qca_soc_reg_read(QCA_LSUART_RBR_REG)& QCA_LSUART_RBR_RBR_MASK) ;
			}
		}
	}
}

static rt_err_t rt_qca953x_lsuart_init (rt_device_t dev)
{
    qca953x_lsuart_init();
	
	qca953x_interrupt_install(QCA953X_UART_INT, rt_qca953x_lsuart_irq_handler,0, "qca953x_lsuart_handler");
	
    qca953x_interrupt_mask(QCA953X_UART_INT);
    return RT_EOK;
}

int soc_console_init(void)
{
	/* device initialization */
	soc_console_dev.type = RT_Device_Class_Char;

	/* device interface */
	soc_console_dev.init 	   = rt_qca953x_lsuart_init;
	soc_console_dev.open 	   = rt_qca953x_lsuart_open;
	soc_console_dev.close      = rt_qca953x_lsuart_close;
	soc_console_dev.read 	   = rt_qca953x_lsuart_read;
	soc_console_dev.write      = rt_qca953x_lsuart_write;
	soc_console_dev.control    = RT_NULL;
	soc_console_dev.user_data  = RT_NULL;
	soc_console_dev.rx_indicate = RT_NULL;
    soc_console_dev.tx_complete = RT_NULL;

	rt_device_register(&soc_console_dev, RT_CONSOLE_DEVICE_NAME, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_INT_RX);
	
	return 0;
}
