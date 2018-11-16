/*
 * SOC common registers and helper function/macro definitions
 *
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _AR933X_CLOCK_H_
#define _AR933X_CLOCK_H_

rt_uint32_t ar933x_get_xtal_hz(void);
void ar933x_clock_init(void);

/*
 * Useful clock variables
 */
#define VAL_40MHz	(40 * 1000 * 1000)
#define VAL_25MHz	(25 * 1000 * 1000)

#endif /* _SOC_COMMON_H_ */
