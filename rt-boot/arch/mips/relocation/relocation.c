/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <finsh/finsh.h>
#include <net/lwip/lwip/opt.h>
#include <global/global.h>
#include <init/init.h>
#include <arch/cache.h>
#include <arch/relocs.h>

register rt_uint32_t $GP __asm__ ("$28");
register rt_uint32_t $SP __asm__ ("$29");
typedef void (late_init_entry)(void);

static inline void arch_got_relocation(void)
{
    rt_uint32_t i;
    rt_uint32_t got_entries_numbers = &__got_end - &__got_start;
    rt_uint32_t *pointer_32;

    pointer_32 = (rt_uint32_t *)rtboot_data.new_gp;

    for(i=2;i<got_entries_numbers;i++)
    {
        if(pointer_32[i])
        	pointer_32[i] += rtboot_data.relocation_offset;
    }
}

static inline rt_uint32_t read_uint(rt_uint8_t **buf)
{
	rt_uint32_t val = 0;
	rt_uint32_t shift = 0;
	rt_uint8_t new;

	do {
		new = *(*buf)++;
		val |= (new & 0x7f) << shift;
		shift += 7;
	} while (new & 0x80);

	return val;
}

static inline void apply_reloc(rt_uint32_t type, void *addr, rt_uint32_t off)
{
	switch (type) {
	case R_MIPS_32:
		*(rt_uint32_t *)addr += off;
		break;
	default:
		break;
	}
}

static inline void arch_table_relocation(void)
{
	rt_uint32_t addr;
	rt_uint8_t *buf;
	rt_uint32_t type;

	buf = __rel_table_start;
	addr = rtboot_data.relocation_base;
	while (1) {
		type = read_uint(&buf);
		if (type == R_MIPS_NONE)
			break;
		addr += read_uint(&buf) << 2;
		apply_reloc(type, (void *)addr, rtboot_data.relocation_offset);
	}
}

/* Here, we need to relocate the code */
void arch_relocation(void)
{
	register late_init_entry *late_init;
	register rt_uint32_t temp_gp,temp_sp;
	
	late_init = &system_late_init;
	
	rtboot_data.old_gp = $GP;
	
	rtboot_data.new_gp = (rtboot_data.old_gp - CFG_MONITOR_BASE) + rtboot_data.relocation_base;
	
	rtboot_data.relocation_offset = rtboot_data.new_gp - rtboot_data.old_gp;
	
	rt_memcpy((void *)rtboot_data.relocation_base,(void *)CFG_MONITOR_BASE,rtboot_data.rtboot_length);
	
    arch_got_relocation();
	arch_table_relocation();

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
	
