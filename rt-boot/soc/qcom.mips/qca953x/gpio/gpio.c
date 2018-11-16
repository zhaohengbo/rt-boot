/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_gpio.h>

void qca953x_gpio_init(void)
{	
	
	
}

void qca953x_all_led_on(void)
{
	rt_uint32_t temp;
	temp = qca_soc_reg_read(QCA_GPIO_OUT_REG);
	temp &= 0xFFFFDFFF;
	qca_soc_reg_write(QCA_GPIO_OUT_REG,temp);
}

void qca953x_all_led_off(void)
{	
	
	
}
