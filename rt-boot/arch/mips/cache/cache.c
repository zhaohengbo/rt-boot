/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <arch/mipsregs.h>

/* Here, we need to get cache information */
void arch_cache_init(void)
{
	rtboot_data.cp0_config1_raw = read_c0_config1();
	rtboot_data.icache_sets_per_way_raw = (rtboot_data.cp0_config1_raw >> 22) & 0x7;
	rtboot_data.icache_line_size_raw = (rtboot_data.cp0_config1_raw >> 19) & 0x7;
	if(rtboot_data.icache_line_size_raw != 0)
	{
		rtboot_data.icache_associativity_level_raw = (rtboot_data.cp0_config1_raw >> 16) & 0x7;
		/* The cache line size is fixed at 32 bytes when the I-Cache is present */
		rtboot_data.icache_line_size = 2 << (rtboot_data.icache_line_size_raw % 32);
		rtboot_data.icache_associativity_level = rtboot_data.icache_associativity_level_raw + 1;
		/* icache_sets_per_way_raw = 0 means sets = 64 and so on, but if it is 7 ,sets = 32*/
		if(rtboot_data.icache_sets_per_way_raw != 0x7)
			rtboot_data.icache_size = (64 << (rtboot_data.icache_sets_per_way_raw % 8)) * rtboot_data.icache_line_size * rtboot_data.icache_associativity_level;
		else
			rtboot_data.icache_size = 32 * rtboot_data.icache_line_size * rtboot_data.icache_associativity_level;
	}
	else
	{
		rtboot_data.icache_associativity_level_raw = 0;
		rtboot_data.icache_line_size = 0;
		rtboot_data.icache_associativity_level = 0;
		rtboot_data.icache_size = 0;
	}
	
	rtboot_data.dcache_sets_per_way_raw = (rtboot_data.cp0_config1_raw >> 13) & 0x7;
	rtboot_data.dcache_line_size_raw = (rtboot_data.cp0_config1_raw >> 10) & 0x7;
	if(rtboot_data.dcache_line_size_raw != 0)
	{
		rtboot_data.dcache_associativity_level_raw = (rtboot_data.cp0_config1_raw >> 7) & 0x7;
		/* The cache line size is fixed at 32 bytes when the D-Cache is present */
		rtboot_data.dcache_line_size = 2 << (rtboot_data.dcache_line_size_raw % 32);
		rtboot_data.dcache_associativity_level = rtboot_data.dcache_associativity_level_raw + 1;
		/* dcache_sets_per_way_raw = 0 means sets = 64 and so on, but if it is 7 ,sets = 32*/
		if(rtboot_data.dcache_sets_per_way_raw != 0x7)
			rtboot_data.dcache_size = (64 << (rtboot_data.dcache_sets_per_way_raw % 8)) * rtboot_data.dcache_line_size * rtboot_data.dcache_associativity_level;
		else
			rtboot_data.dcache_size = 32 * rtboot_data.dcache_line_size * rtboot_data.dcache_associativity_level;
	}
	else
	{
		rtboot_data.dcache_associativity_level_raw = 0;
		rtboot_data.dcache_line_size = 0;
		rtboot_data.dcache_associativity_level = 0;
		rtboot_data.dcache_size = 0;
	}
}

void arch_dcache_flush(rt_uint32_t start_addr,rt_uint32_t size)
{
	rt_uint32_t end_addr, temp_addr;
	if(rtboot_data.dcache_size != 0)
	{
		temp_addr = start_addr & ~(rtboot_data.dcache_line_size - 1);
		size = (size + rtboot_data.dcache_line_size - 1) & ~(rtboot_data.dcache_line_size - 1);
		end_addr = temp_addr + size;
		for(;temp_addr < end_addr;temp_addr += rtboot_data.dcache_line_size)
		{
			__asm__ __volatile__("	cache   0x15, 0(%0)" :: "r" (temp_addr));
		}
	}
}

void arch_dcache_invalidate(rt_uint32_t start_addr,rt_uint32_t size)
{
	rt_uint32_t end_addr, temp_addr;
	if(rtboot_data.dcache_size != 0)
	{
		temp_addr = start_addr & ~(rtboot_data.dcache_line_size - 1);
		size = (size + rtboot_data.dcache_line_size - 1) & ~(rtboot_data.dcache_line_size - 1);
		end_addr = temp_addr + size;
		for(;temp_addr < end_addr;temp_addr += rtboot_data.dcache_line_size)
		{
			__asm__ __volatile__("	cache   0x11, 0(%0)" :: "r" (temp_addr));
		}
	}
}

void arch_icache_flush(rt_uint32_t start_addr,rt_uint32_t size)
{
	rt_uint32_t end_addr, temp_addr;
	if(rtboot_data.icache_size != 0)
	{
		temp_addr = start_addr & ~(rtboot_data.icache_line_size - 1);
		size = (size + rtboot_data.icache_line_size - 1) & ~(rtboot_data.icache_line_size - 1);
		end_addr = temp_addr + size;
		for(;temp_addr < end_addr;temp_addr += rtboot_data.icache_line_size)
		{
			__asm__ __volatile__("	cache   0x10, 0(%0)" :: "r" (temp_addr));
		}
	}
}

void arch_cache_flush(rt_uint32_t start_addr,rt_uint32_t size)
{
	rt_uint32_t end_addr, temp_addr;
	if((rtboot_data.icache_size != 0) && (rtboot_data.dcache_size != 0))
	{
		if(rtboot_data.icache_line_size == rtboot_data.dcache_line_size)
		{
			temp_addr = start_addr & ~(rtboot_data.icache_line_size - 1);
			size = (size + rtboot_data.icache_line_size - 1) & ~(rtboot_data.icache_line_size - 1);
			end_addr = temp_addr + size;
			for(;temp_addr < end_addr;temp_addr += rtboot_data.icache_line_size)
			{
				__asm__ __volatile__("	cache   0x15, 0(%0)" :: "r" (temp_addr));
				__asm__ __volatile__("	cache   0x10, 0(%0)" :: "r" (temp_addr));
			}
		}
		else
		{
			arch_dcache_flush(start_addr,size);
			arch_icache_flush(start_addr,size);
		}
	}
}
