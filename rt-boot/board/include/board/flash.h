/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _BORAD_FLASH_H_
#define _BORAD_FLASH_H_

rt_err_t board_flash_init(void);
rt_int32_t board_flash_get_status(void);
void board_flash_firmware_notisfy(void);
void board_flash_uboot_notisfy(void);
void board_flash_art_notisfy(void);
void board_flash_full_notisfy(void);
void board_flash_error_notisfy(void);

rt_uint32_t board_get_env_flag(void);
rt_uint32_t board_get_env_length(void);
void board_get_env_read(rt_uint8_t * buffer);
void board_get_env_write(rt_uint8_t * buffer);

#endif /* _SOC_COMMON_H_ */
