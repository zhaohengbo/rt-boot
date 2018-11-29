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
#include <drivers/drivers/spi_flash.h>
#include <drivers/drivers/spi_flash_sfud.h>

#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>

#define SPI_BUS_NAME                "SPI0"
#define SPI_FLASH_DEVICE_NAME       "FLASH0"
#define SPI_FLASH_CHIP              "SPI-FLASH0"

static struct rt_spi_device spi_flash_device;
static rt_spi_flash_device_t flash_chip_device;

static rt_err_t rt_hw_spi_flash_attach(void)
{
    /* attach cs */
	rt_err_t result;

    result = rt_spi_bus_attach_device(&spi_flash_device, SPI_FLASH_DEVICE_NAME, SPI_BUS_NAME, (void*)0);
	if (result != RT_EOK)
	{
		return result;
	}

	return RT_EOK;
}

rt_err_t soc_flash_init(void)
{
	rt_err_t result;
	result = rt_hw_spi_flash_attach();
	if (result != RT_EOK)
	{
		return result;
	}
    flash_chip_device = rt_sfud_flash_probe(SPI_FLASH_CHIP, SPI_FLASH_DEVICE_NAME);
    if (flash_chip_device == RT_NULL)
		return RT_ERROR;
	return RT_EOK;
}
