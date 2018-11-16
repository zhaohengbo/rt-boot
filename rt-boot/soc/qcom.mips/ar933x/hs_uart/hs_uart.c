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
 * Linux/drivers/tty/serial/ar933x_uart.c
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <soc/ar933x/ar933x_map.h>
#include <soc/ar933x/ar933x_clock.h>
#include <soc/ar933x/ar933x_hs_uart.h>
#include <soc/ar933x/ar933x_gpio.h>

#define CONFIG_BAUDRATE 115200

/* HS UART baudrate = (REF_CLK / (CLOCK_SCALE + 1)) * (CLOCK_STEP * (1 / 2^17)) */
static rt_uint32_t ar933x_hsuart_get_baud(rt_uint32_t ref_clk, rt_uint32_t uart_scale, rt_uint32_t uart_step)
{
	rt_uint64_t baudrate;
	rt_uint32_t div;

	div = (uart_scale + 1) * (2 << 16);

	baudrate = (ref_clk * uart_step) + (div / 2);
	baudrate = baudrate / div;

	return (rt_uint32_t)baudrate;
}

static void ar933x_hsuart_get_scale_step(rt_uint32_t baudrate,
									  rt_uint32_t *uart_scale,
									  rt_uint32_t *uart_step)
{
	rt_int32_t diff;
	rt_uint32_t ref_clk;
	rt_uint32_t tscale;
	rt_uint64_t tstep;
	rt_int32_t min_diff;

	*uart_scale = 0;
	*uart_step = 0;

	min_diff = baudrate;

	ref_clk = ar933x_get_xtal_hz();

	for (tscale = 0; tscale < QCA_HSUART_CLK_SCALE_MAX_VAL; tscale++) {
		tstep = baudrate * (tscale + 1);
		tstep = tstep * (2 << 16);
		tstep = tstep / ref_clk;

		if (tstep > QCA_HSUART_CLK_STEP_MAX_VAL)
			break;

		diff = ar933x_hsuart_get_baud(ref_clk, tscale, tstep) - baudrate;

		if (diff < 0)
			diff = -1 * diff;

		if (diff < min_diff) {
			min_diff = diff;
			*uart_scale = tscale;
			*uart_step = tstep;
		}
	}
}

static void ar933x_hsuart_setbrg(void)
{
	rt_uint32_t uart_clock;
	rt_uint32_t uart_scale;
	rt_uint32_t uart_step;

	ar933x_hsuart_get_scale_step(CONFIG_BAUDRATE, &uart_scale, &uart_step);

	uart_clock  = (uart_scale << QCA_HSUART_CLK_SCALE_SHIFT);
	uart_clock |= (uart_step  << QCA_HSUART_CLK_STEP_SHIFT);

	qca_soc_reg_write(QCA_HSUART_CLK_REG, uart_clock);
}

void ar933x_hsuart_init(void)
{
	rt_uint32_t uart_cs;

	/*
	 * High-Speed UART controller configuration:
	 * - no DMA
	 * - no interrupt
	 * - no parity
	 * - DCE mode
	 * - no flow control
	 * - set RX ready oride
	 * - set TX ready oride
	 */
	uart_cs = (0 << QCA_HSUART_CS_DMA_EN_SHIFT) |
		(0 << QCA_HSUART_CS_HOST_INT_EN_SHIFT) |
		(1 << QCA_HSUART_CS_RX_READY_ORIDE_SHIFT) |
		(1 << QCA_HSUART_CS_TX_READY_ORIDE_SHIFT) |
		(QCA_HSUART_CS_PAR_MODE_NO_VAL << QCA_HSUART_CS_PAR_MODE_SHIFT) |
		(QCA_HSUART_CS_IFACE_MODE_DCE_VAL << QCA_HSUART_CS_IFACE_MODE_SHIFT) |
		(QCA_HSUART_CS_FLOW_MODE_NO_VAL << QCA_HSUART_CS_FLOW_MODE_SHIFT);

	qca_soc_reg_write(QCA_HSUART_CS_REG, uart_cs);

	ar933x_hsuart_setbrg();
}

void ar933x_hsuart_putc(const char c)
{
	rt_uint32_t uart_data;

	if (c == '\n')
		ar933x_hsuart_putc('\r');

	/* Wait for FIFO */
	do {
		uart_data = qca_soc_reg_read(QCA_HSUART_DATA_REG);
	} while (((uart_data & QCA_HSUART_DATA_TX_CSR_MASK)
			  >> QCA_HSUART_DATA_TX_CSR_SHIFT)  == 0);

	/* Put data in buffer and set CSR bit */
	uart_data  = (rt_uint32_t)c | (1 << QCA_HSUART_DATA_TX_CSR_SHIFT);

	qca_soc_reg_write(QCA_HSUART_DATA_REG, uart_data);
}

int ar933x_hsuart_tstc(void)
{
	rt_uint32_t uart_data = qca_soc_reg_read(QCA_HSUART_DATA_REG);

	return ((uart_data & QCA_HSUART_DATA_RX_CSR_MASK)
			>> QCA_HSUART_DATA_RX_CSR_SHIFT);
}

int ar933x_hsuart_getc(void)
{
	rt_uint32_t uart_data;

	while (!ar933x_hsuart_tstc())
		;

	uart_data = qca_soc_reg_read(QCA_HSUART_DATA_REG);

	qca_soc_reg_write(QCA_HSUART_DATA_REG,
					  (1 << QCA_HSUART_DATA_RX_CSR_SHIFT));

	return (uart_data & QCA_HSUART_DATA_TX_RX_DATA_MASK);
}

void ar933x_hsuart_puts(const char *s)
{
	while (*s)
		ar933x_hsuart_putc(*s++);
}
