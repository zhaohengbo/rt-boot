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
#include <drivers/rtdevice.h>
#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_spi.h>

static struct rt_spi_bus qca953x_spi_bus;
static struct rt_spi_ops qca953x_spi_ops;

static rt_uint32_t qca953x_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
	rt_uint32_t cs_bank = (rt_uint32_t)device->parent.user_data;
    if (message->cs_take)
	{
		qca953x_spi_enable();
		qca953x_change_cs(cs_bank);
	}
    rt_uint8_t *sndb = (rt_uint8_t *)message->send_buf;
    rt_uint8_t *rcvb = message->recv_buf;
    rt_int32_t length = message->length;
	
    while (length--)
    {
		if(sndb != RT_NULL)
        	qca953x_shift_out(*sndb++,8,0);
		else
			qca953x_shift_out(0,8,0);
		if(rcvb != RT_NULL)
			*rcvb++ = qca953x_shift_in();
    }
    if (message->cs_release)
    {
        qca953x_shift_out(0,0,1);
		qca953x_spi_disable();
    }
    return message->length - length;
}

static rt_err_t qca953x_spi_configure(struct rt_spi_device *device,
                       struct rt_spi_configuration *configuration)
{
	return RT_EOK;
}

rt_err_t soc_spi_init(void)
{
	qca953x_spi_ops.configure = qca953x_spi_configure;
	qca953x_spi_ops.xfer = qca953x_spi_xfer;
	return rt_spi_bus_register(&qca953x_spi_bus, "SPI0", &qca953x_spi_ops);
}