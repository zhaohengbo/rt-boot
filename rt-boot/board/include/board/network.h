/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _BORAD_NETWORK_H_
#define _BORAD_NETWORK_H_

void board_network_init(void);
void board_network_start(void);

void board_eth_recv_register_cb(void (*cb)(void *para),void *para);

void board_eth_recv(rt_uint8_t *ip_buffer,rt_uint32_t *ip_length);
void board_eth_send(rt_uint8_t *ip_buffer,rt_uint32_t ip_length);

#endif /* _SOC_COMMON_H_ */
