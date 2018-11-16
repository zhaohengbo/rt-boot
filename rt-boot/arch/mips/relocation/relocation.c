/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <finsh/finsh.h>
#include <common/global.h>
#include <common/init.h>
#include <arch/cache.h>

register rt_uint32_t $GP __asm__ ("$28");
register rt_uint32_t $SP __asm__ ("$29");
typedef void (late_init_entry)(void);

/* Here, we need to relocate the code */
void arch_relocation(void)
{
	rt_uint32_t i;
	rt_uint32_t got_entries_numbers = &__got_end - &__got_start;
	rt_uint32_t fsymtab_numbers = &__fsymtab_end - &__fsymtab_start;
	rt_uint32_t vsymtab_numbers = &__vsymtab_end - &__vsymtab_start;
	rt_uint32_t rtobject_numbers = &__rtobject_end - &__rtobject_start;
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
	
	pointer_32 = (rt_uint32_t *)((rt_uint32_t)&__rtobject_start + rtboot_data.relocation_offset);
	
	for(i=0;i<rtobject_numbers;i++)
	{
		if(((i%4) == 1) | ((i%4) == 2))
			pointer_32[i] += rtboot_data.relocation_offset;
	}
	
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
	