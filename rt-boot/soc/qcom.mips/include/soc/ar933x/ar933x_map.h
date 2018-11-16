/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _AR933X_MAP_H_
#define _AR933X_MAP_H_

#include <arch/addrspace.h>

/*
 * Helper macros
 */
#define BIT(_x)					(1 << (_x))
#define BITS(_start, _bits)		(((1 << (_bits)) - 1) << _start)
#define CHECK_BIT(_var, _pos)	((_var) & (1 << (_pos)))

/*
 * Address map
 */
#define QCA_APB_BASE_REG	0x18000000
#define QCA_FLASH_BASE_REG	0x1F000000
#define QCA_PCIE_BASE_REG	0x10000000

/*
 * APB block
 */
#define QCA_DDR_CTRL_BASE_REG		QCA_APB_BASE_REG + 0x00000000
#define QCA_HSUART_BASE_REG	QCA_APB_BASE_REG + 0x00020000
#define QCA_USB_CFG_BASE_REG		QCA_APB_BASE_REG + 0x00030000
#define QCA_GPIO_BASE_REG		QCA_APB_BASE_REG + 0x00040000
#define QCA_PLL_BASE_REG		QCA_APB_BASE_REG + 0x00050000
#define QCA_RST_BASE_REG		QCA_APB_BASE_REG + 0x00060000
#define QCA_GMAC_BASE_REG		QCA_APB_BASE_REG + 0x00070000
#define QCA_RTC_BASE_REG		QCA_APB_BASE_REG + 0x00107000
#define QCA_PLL_SRIF_BASE_REG		QCA_APB_BASE_REG + 0x00116000
#define QCA_PCIE_RC0_CTRL_BASE_REG	QCA_APB_BASE_REG + 0x000F0000
#define QCA_PCIE_RC1_CTRL_BASE_REG	QCA_APB_BASE_REG + 0x00280000
#define QCA_SLIC_BASE_REG	QCA_APB_BASE_REG + 0x00090000

/*
 * Read, write, set and clear macros
 */
#define qca_soc_reg_read(_addr)		\
		*(volatile unsigned int *)(KSEG1ADDR(_addr))

#define qca_soc_reg_write(_addr, _val)	\
		((*(volatile unsigned int *)KSEG1ADDR(_addr)) = (_val))

#define qca_soc_reg_read_set(_addr, _mask)	\
		qca_soc_reg_write((_addr), (qca_soc_reg_read((_addr)) | (_mask)))

#define qca_soc_reg_read_clear(_addr, _mask)	\
		qca_soc_reg_write((_addr), (qca_soc_reg_read((_addr)) & ~(_mask)))

#endif /* _SOC_COMMON_H_ */
