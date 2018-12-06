/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _BORAD_FLASH_H_
#define _BORAD_FLASH_H_

rt_err_t soc_flash_init(void);
rt_int32_t board_flash_get_status(void);
void board_flash_firmware_notisfy(void);

#endif /* _SOC_COMMON_H_ */
