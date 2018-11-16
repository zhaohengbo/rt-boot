/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <soc/ar933x/ar933x_map.h>
#include <soc/ar933x/ar933x_clock.h>
#include <soc/ar933x/ar933x_reset.h>

rt_uint32_t ar933x_get_xtal_hz(void)
{
	if ((qca_soc_reg_read(QCA_RST_BOOTSTRAP_REG) & 
		 QCA_RST_BOOTSTRAP_REF_CLK_MASK) >> QCA_RST_BOOTSTRAP_REF_CLK_SHIFT)
	{
		return VAL_40MHz;
	}
	else
	{
		return VAL_25MHz;
	}
}

void ar933x_clock_init(void)
{	
	
	
}
