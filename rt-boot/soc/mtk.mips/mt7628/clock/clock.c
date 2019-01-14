/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/mt7628/mt7628_mmap.h>
#include <soc/mt7628/mt7628_clock.h>

void mt7628_clock_init(void)
{
	rt_uint32_t reg;
	rt_uint32_t mips_cpu_feq,mips_bus_feq;
	reg = __REG(RALINK_CLKCFG0_REG);
	if (reg & (0x1<<1)) {
		mips_cpu_feq = (480*1000*1000)/CPU_FRAC_DIV;
	}else if (reg & 0x1) {
		mips_cpu_feq = ((__REG(RALINK_SYS_CGF0_REG)>>6)&0x1) ? (40*1000*1000)/CPU_FRAC_DIV \
					   : (25*1000*1000)/CPU_FRAC_DIV;
	}else {
		if ((__REG(RALINK_SYS_CGF0_REG)>>6)&0x1)
			mips_cpu_feq = (580*1000*1000)/CPU_FRAC_DIV;
		else
			mips_cpu_feq = (575*1000*1000)/CPU_FRAC_DIV;
	}
	mips_bus_feq = mips_cpu_feq/3;
	
	rtboot_data.cpu_clk = mips_cpu_feq;
	rtboot_data.bus_clk = mips_bus_feq;
	
	rtboot_data.system_frequency = rtboot_data.cpu_clk;
}
