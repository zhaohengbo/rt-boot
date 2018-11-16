/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <common/global.h>
#include <arch/mipsregs.h>
#include <arch/cache.h>
#include <arch/vector.h>
#include <arch/coprocessor.h>

register rt_uint32_t $GP __asm__ ("$28");

/* Here, we need to part the memory */
void arch_early_init(void)
{	
	rtboot_data.rtboot_length = (rt_uint32_t)&rtboot_end - CFG_MONITOR_BASE;
	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM
	 */
	rtboot_data.relocation_base = CFG_SDRAM_BASE + rtboot_data.system_memsize;
	/*
	 * We can reserve some RAM "on top" here,
	 * round down to next 4 kB limit
	 */
	rtboot_data.relocation_base &= ~(4096 - 1);
	/*
	 * Reserve memory for U-Boot code, data & bss,
	 * round down to next 16 kB limit
	 */
	rtboot_data.relocation_base -= rtboot_data.rtboot_length;
	rtboot_data.relocation_base &= ~(16 * 1024 - 1);
	/*
	 * Reserve memory for Interrupt Vector,
	 * round down to next 4 kB limit
	 */
	
	rtboot_data.start_stack_base = rtboot_data.relocation_base - 0x1000;
	rtboot_data.start_stack_base &= ~(4096 - 1);
	rtboot_data.relocation_ebase = rtboot_data.start_stack_base;
	
	/* Reserve memory for malloc() arena */
	
	rtboot_data.malloc_end_base = rtboot_data.start_stack_base - 4;
	
	rtboot_data.start_stack_base -= TOTAL_MALLOC_LEN;
	rtboot_data.start_stack_base &= ~(4096 - 1);
	
	rtboot_data.malloc_start_base = rtboot_data.start_stack_base;
	
	/*
	 * Reserve memory for boot params
	 */
	rtboot_data.start_stack_base -= CFG_BOOTPARAMS_LEN;
	
	rtboot_data.boot_param_pointer = rtboot_data.start_stack_base;

	/* increase by mistake gcc at last */
	/*
	 * This is important as in relocation newsp will be 
	 * increase by gcc's 'mistake' at last
	 */
	
	rtboot_data.start_stack_base;
	
	rtboot_data.start_stack_base -= 0x10;
	
	rtboot_data.start_stack_base &= ~0xF;
	
	rtboot_data.start_stack_base -= 8;
	
	rt_memset((void *)(rtboot_data.start_stack_base),0,0x8);
	
	arch_cache_init();
}

void arch_late_init(void)
{
	arch_cache_init();
	
	/* clean memory */
	rt_memset((void *)CFG_MONITOR_BASE,0,rtboot_data.rtboot_length);
	arch_cache_flush(CFG_MONITOR_BASE,rtboot_data.rtboot_length);
	
	/* init vector */
	mips_vector_init();
	
	/* init hardware interrupt */
	rt_hw_interrupt_init();
	
	mips_cp0_init();
	
#ifdef RT_USING_HEAP
	rt_system_heap_init((void*)rtboot_data.malloc_start_base, (void*)rtboot_data.malloc_end_base);
#endif

#ifdef RT_USING_FPU
    /* init hardware fpu */
    rt_hw_fpu_init();
#endif
}
	