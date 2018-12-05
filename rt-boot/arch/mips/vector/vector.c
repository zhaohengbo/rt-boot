#include <kernel/rtthread.h>
#include <kernel/rthw.h>

#include <global/global.h>
#include <arch/mipsregs.h>
#include <arch/cache.h>
#include <arch/addrspace.h>
#include <arch/coprocessor.h>

extern void mips_irq_handle(void);
extern void  mips_ecpt_handle(void);
extern void  mips_tlb_handle(void);
extern void  mips_cache_handle(void);

register rt_uint32_t $GP __asm__ ("$28");

const rt_uint32_t jump_program[]=
{
	0x3C1A0000,//li k0 0x00000000
	0x375A0000,
	0x3C1B0000,//li k1 0x00000000
	0x377B0000,
	
	0x03600008,//jr k1
	0x00000000 //nop
};
	

static void mips_vector_fill(rt_uint32_t address,rt_uint32_t pc_pointer)
{
	rt_uint32_t gp_data;
	rt_uint32_t idx;
	rt_uint32_t *pt;
	gp_data = $GP;
	
	pt = (rt_uint32_t *)address;
	
	for(idx=0;idx<(sizeof(jump_program) / sizeof(rt_uint32_t));idx++)
	{
		pt[idx] = jump_program[idx];
	}
	
	pt[0] |= (gp_data&0xFFFF0000) >> 16;
	
	pt[1] |= (gp_data&0x0000FFFF);	
	
	pt[2] |= (pc_pointer&0xFFFF0000) >> 16;
	
	pt[3] |= (pc_pointer&0x0000FFFF);
	
}

void mips_vector_init(void)
{
	rt_uint32_t new_ebase;
	
	if(mips_cpu_version() == MIPS32R2)
		new_ebase = rtboot_data.relocation_ebase;
	else
		new_ebase = K0BASE;
	
	write_c0_ebase(new_ebase);
	
	rt_memset((void *)new_ebase,0,(0x1000-4));
	
	mips_vector_fill(new_ebase,(rt_uint32_t)&mips_tlb_handle);
	mips_vector_fill(new_ebase + 0x100,KSEG1ADDR((rt_uint32_t)&mips_cache_handle));
	mips_vector_fill(new_ebase + 0x180,(rt_uint32_t)&mips_ecpt_handle);
	mips_vector_fill(new_ebase + 0x200,(rt_uint32_t)&mips_irq_handle);
	
	arch_cache_flush(new_ebase,(0x1000-4));
}

void mips_vector_restore(void)
{
	if(mips_cpu_version() == MIPS32R2)
		write_c0_ebase(K0BASE);
}