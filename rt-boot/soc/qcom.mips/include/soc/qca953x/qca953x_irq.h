/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _QCA953X_IRQ_H_
#define _QCA953X_IRQ_H_

enum qca953x_irq_type {
	QCA953X_TIMER_INT = 0,
	QCA953X_ERROR_INT,
	QCA953X_GPIO_INT,
	QCA953X_UART_INT,
	QCA953X_WATCHDOG_INT,
	QCA953X_PC_INT,
	QCA953X_RES1_INT,
	QCA953X_RES2_INT,
	QCA953X_TIMER2_INT,
	QCA953X_TIMER3_INT,
	QCA953X_TIMER4_INT,
	QCA953X_RES3_INT,
	QCA953X_SW_MAC_INT_INT,
	QCA953X_RES4_INT,
	QCA953X_DDR_SF_ENTRY_INT,
	QCA953X_DDR_SF_EXIT_INT,
	QCA953X_DDR_ACTIVITY_IN_SF_INT,
	QCA953X_RESN_INT
};

void qca953x_interrupt_mask(int vector);
void qca953x_interrupt_umask(int vector);
rt_isr_handler_t qca953x_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, char *name);
void qca953x_interrupt_init(void);

#endif /* _SOC_COMMON_H_ */
