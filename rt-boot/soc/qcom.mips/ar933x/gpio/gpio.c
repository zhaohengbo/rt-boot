/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/ar933x/ar933x_map.h>
#include <soc/ar933x/ar933x_gpio.h>

void ar933x_gpio_init(void)
{	
	
	
}

void ar933x_all_led_on(void)
{
	rt_uint32_t temp;
	temp = qca_soc_reg_read(QCA_GPIO_OUT_REG);
	//temp &= 0xFFFFDFFF;
	temp |= 0x1E003;
	temp &= 0xF7FDFFFF;
	qca_soc_reg_write(QCA_GPIO_OUT_REG,temp);
}

void ar933x_all_led_off(void)
{	
	
	
}
