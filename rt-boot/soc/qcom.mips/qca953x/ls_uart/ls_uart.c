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
#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_clock.h>
#include <soc/qca953x/qca953x_ls_uart.h>
#include <soc/qca953x/qca953x_gpio.h>

static void qca953x_lsuart_setbrg(rt_uint32_t baudrate)
{
	rt_uint32_t div;

	/*
	 * TODO: prepare list of supported range of baudrate values
	 * For 40 MHz ref_clk, successfully tested up to 1152000 on AR9344
	 */

	/* Round to closest, final baudrate = ref_clk / (16 * div) */
	div = (qca953x_get_xtal_hz() + (8 * baudrate)) / (16 * baudrate);

	/* Set DLAB bit in LCR register unlocks DLL/DLH registers */
	qca_soc_reg_read_set(QCA_LSUART_LCR_REG, QCA_LSUART_LCR_DLAB_MASK);

	/* Write div into DLL and DLH registers */
	qca_soc_reg_write(QCA_LSUART_DLL_REG, (div & 0xFF));
	qca_soc_reg_write(QCA_LSUART_DLH_REG, ((div >> 8) & 0xFF));

	/* Clear DLAB bit in LCR register */
	qca_soc_reg_read_clear(QCA_LSUART_LCR_REG, QCA_LSUART_LCR_DLAB_MASK);
}

void qca953x_lsuart_init(void)
{
	qca953x_lsuart_setbrg(rtboot_data.system_baudrate);

	/* No interrupt */
	qca_soc_reg_write(QCA_LSUART_IER_REG, 0x0);

	/* Enable FIFO/No DMA */
	qca_soc_reg_write(QCA_LSUART_FCR_REG, QCA_LSUART_FCR_EDDSI_MASK | QCA_LSUART_FCR_RCVR_FIFO_RST_MASK | QCA_LSUART_FCR_XMIT_FIFO_RST_MASK);

	/*
	 * Low-Speed UART controller configuration:
	 * - data: 8bits
	 * - stop: 1bit
	 * - parity: no
	 */
	qca_soc_reg_write(QCA_LSUART_LCR_REG,
					  QCA_LSUART_LCR_CLS_8BIT_VAL << QCA_LSUART_LCR_CLS_SHIFT);
}

void qca953x_lsuart_putc(const char c)
{
	rt_uint32_t line_status;

	if (c == '\n')
		qca953x_lsuart_putc('\r');

	/* Wait for empty THR */
	do {
		line_status = qca_soc_reg_read(QCA_LSUART_LSR_REG)
					  & QCA_LSUART_LSR_THRE_MASK;
	} while (line_status == 0);

	/* Put data in THR */
	qca_soc_reg_write(QCA_LSUART_THR_REG, (rt_uint32_t)c);
}

int qca953x_lsuart_getc(void)
{
	while (!qca953x_lsuart_tstc())
		;

	/* Get data from RBR */
	return qca_soc_reg_read(QCA_LSUART_RBR_REG)
		   & QCA_LSUART_RBR_RBR_MASK;
}

int qca953x_lsuart_tstc(void)
{
	/* Check data ready bit */
	return qca_soc_reg_read(QCA_LSUART_LSR_REG)
		   & QCA_LSUART_LSR_DR_MASK;
}

void qca953x_lsuart_puts(const char *s)
{
	while (*s)
		qca953x_lsuart_putc(*s++);
}
