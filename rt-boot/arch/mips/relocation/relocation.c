/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <finsh/finsh.h>
#include <net/lwip/lwip/opt.h>
#include <common/global.h>
#include <common/init.h>
#include <arch/cache.h>

register rt_uint32_t $GP __asm__ ("$28");
register rt_uint32_t $SP __asm__ ("$29");
typedef void (late_init_entry)(void);

static void rt_thread_relocation(void)
{
	rt_uint32_t i;
	rt_uint32_t rtobject_numbers = &__rtobject_end - &__rtobject_start;
	rt_uint32_t *pointer_32;
	
	pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__rtobject_start + rtboot_data.relocation_offset);
	
	for(i=0;i<rtobject_numbers;i++)
	{
		if(((i%4) == 1) | ((i%4) == 2))
			pointer_32[i] += rtboot_data.relocation_offset;
	}
}

static void finsh_relocation(void)
{
	rt_uint32_t i;
	rt_uint32_t fsymtab_numbers = &__fsymtab_end - &__fsymtab_start;
	rt_uint32_t vsymtab_numbers = &__vsymtab_end - &__vsymtab_start;
	rt_uint32_t *pointer_32;
	
	pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__fsymtab_start + rtboot_data.relocation_offset);
	
	for(i=0;i<fsymtab_numbers;i++)
	{
		pointer_32[i] += rtboot_data.relocation_offset;
	}
	
	pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__vsymtab_start + rtboot_data.relocation_offset);

	for(i=0;i<vsymtab_numbers;i++)
	{
#if defined(FINSH_USING_DESCRIPTION) && defined(FINSH_USING_SYMTAB)
		if((i%4) != 2)
#else
		if((i%3) != 1)
#endif
			pointer_32[i] += rtboot_data.relocation_offset;
	}
}

static void lwip_relocation(void)
{
	rt_uint32_t i;
    rt_uint32_t lwip_pcb_list_numbers = &__lwip_tcp_pcb_list_end - &__lwip_tcp_pcb_list_start;
    rt_uint32_t lwip_tcp_state_str_numbers = &__lwip_tcp_state_str_end - &__lwip_tcp_state_str_start;
    rt_uint32_t lwip_err_str_numbers = &__lwip_err_str_end - &__lwip_err_str_start;
    rt_uint32_t lwip_altcp_tcp_function_numbers = &__lwip_altcp_tcp_function_end - &__lwip_altcp_tcp_function_start;

    rt_uint32_t lwip_cyclic_timer_numbers = &__lwip_cyclic_timer_end - &__lwip_cyclic_timer_start;
    rt_uint32_t lwip_mempool_desc_numbers = &__lwip_mempool_desc_end - &__lwip_mempool_desc_start;
    rt_uint32_t lwip_mempool_numbers = &__lwip_mempool_end - &__lwip_mempool_start;
	rt_uint32_t *pointer_32;

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_tcp_pcb_list_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_pcb_list_numbers;i++)
    {
        pointer_32[i] += rtboot_data.relocation_offset;
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_tcp_state_str_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_tcp_state_str_numbers;i++)
    {
        pointer_32[i] += rtboot_data.relocation_offset;
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_err_str_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_err_str_numbers;i++)
    {
        pointer_32[i] += rtboot_data.relocation_offset;
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_altcp_tcp_function_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_altcp_tcp_function_numbers;i++)
    {
        pointer_32[i] += rtboot_data.relocation_offset;
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_cyclic_timer_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_cyclic_timer_numbers;i++)
    {
#if defined(LWIP_DEBUG_TIMERNAMES) || (defined(LWIP_DEBUG) && SYS_DEBUG)
        if((i%3) != 0)
            pointer_32[i] += rtboot_data.relocation_offset;
#else
      if((i%2) != 0)
          pointer_32[i] += rtboot_data.relocation_offset;
#endif
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_mempool_desc_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_mempool_desc_numbers;i++)
    {
#if MEMP_MEM_MALLOC
#if defined(LWIP_DEBUG) || MEMP_OVERFLOW_CHECK || LWIP_STATS_DISPLAY
#if MEMP_STATS
        if((i%3) != 2)
            pointer_32[i] += rtboot_data.relocation_offset;
#else
        if((i%2) == 0)
            pointer_32[i] += rtboot_data.relocation_offset;
#endif
#else
#if MEMP_STATS
        if((i%2) == 0)
            pointer_32[i] += rtboot_data.relocation_offset;
#endif
#endif
#else
#error not supported yet!
#endif
    }

    pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__lwip_mempool_start + rtboot_data.relocation_offset);

    for(i=0;i<lwip_mempool_numbers;i++)
    {
        pointer_32[i] += rtboot_data.relocation_offset;
    }
    //memp_pools[i]->desc = (char *)((rt_uint32_t)memp_pools[i]->desc + rtboot_data.relocation_offset);
}

/* Here, we need to relocate the code */
void arch_relocation(void)
{
	rt_uint32_t i;
	rt_uint32_t got_entries_numbers = &__got_end - &__got_start;
	rt_uint32_t *pointer_32;

	register late_init_entry *late_init;
	register rt_uint32_t temp_gp,temp_sp;
	
	late_init = &system_late_init;
	
	rtboot_data.old_gp = $GP;
	
	rtboot_data.new_gp = (rtboot_data.old_gp - CFG_MONITOR_BASE) + rtboot_data.relocation_base;
	
	rtboot_data.relocation_offset = rtboot_data.new_gp - rtboot_data.old_gp;
	
	rt_memcpy((void *)rtboot_data.relocation_base,(void *)CFG_MONITOR_BASE,rtboot_data.rtboot_length);
	
	pointer_32 = (rt_uint32_t *)rtboot_data.new_gp;
	
	for(i=2;i<got_entries_numbers;i++)
	{
		pointer_32[i] += rtboot_data.relocation_offset;
	}
	
	rt_thread_relocation();
	finsh_relocation();
    lwip_relocation();

	rt_memset((void *)((rt_uint32_t)&rtboot_data_end + rtboot_data.relocation_offset),0,(rt_uint32_t)&rtboot_end - (rt_uint32_t)&rtboot_data_end);
	
	late_init = (late_init_entry *)((rt_uint32_t)late_init + rtboot_data.relocation_offset);
	
	rt_memcpy((void *)((rt_uint32_t)&rtboot_data + rtboot_data.relocation_offset),(void *)&rtboot_data,sizeof(system_boot_data));
	
	arch_cache_flush(rtboot_data.relocation_base,rtboot_data.rtboot_length);
	
	temp_gp = rtboot_data.new_gp;
	temp_sp = rtboot_data.start_stack_base;

	/* some c compiler will do something else when simply imply in c */
#if 1
	$GP = temp_gp;
	$SP = temp_sp;
	late_init();
#else
	__asm__ __volatile__("	move $28,%0" :: "r" (temp_gp));
	__asm__ __volatile__("	move $29,%0" :: "r" (temp_sp));
	__asm__ __volatile__("	move $25,%0" :: "r" (late_init));
	__asm__ __volatile__("	jalr $25");
	__asm__ __volatile__("	nop");
#endif
	
	while(1)
		;//halt system
}
	
