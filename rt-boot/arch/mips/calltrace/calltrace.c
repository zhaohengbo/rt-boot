/*
 * File      : calltrace.c
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <arch/addrspace.h>
#include <arch/stack.h>
#include <finsh/finsh.h>

#define ADDUI_SP_INST           0x27bd0000
#define SW_RA_INST              0xafbf0000
#define JR_RA_INST              0x03e00008

#define INST_OP_MASK            0xffff0000
#define INST_OFFSET_MASK        0x0000ffff

#define abs(s) ((s) < 0 ? -(s):(s))

rt_int32_t calltrace_irq(rt_uint32_t irq_sp)
{
	rt_uint32_t *addr;
	rt_uint32_t *pc, *ra, *sp;
	rt_size_t ra_offset;
	rt_size_t stack_size;
	rt_int32_t depth;
	rt_int32_t size = 8;
	arch_stack_frame *stack_frame = (arch_stack_frame *)irq_sp;

	pc = (rt_uint32_t *)stack_frame->pc;
	ra = (rt_uint32_t *)stack_frame->ra;
	sp = (rt_uint32_t *)stack_frame->sp;

	rt_kprintf("Calltrace:[0x%08x]\n", (rt_uint32_t)pc);

	if (size == 1) return 1;

	ra_offset = stack_size = 0;

	for (addr = ra; !ra_offset || !stack_size; --addr)
	{
		switch (*addr & INST_OP_MASK) {
			case ADDUI_SP_INST:
				stack_size = abs((short)(*addr&INST_OFFSET_MASK));
				break;

			case SW_RA_INST:
				ra_offset = (short)(*addr&INST_OFFSET_MASK);
				break;

			case 0x3c1c0000:
				goto out_of_loop;

			default:
				break;
		}
	}

out_of_loop:
	if (ra_offset)  ra = *(rt_uint32_t **)((rt_uint32_t)sp + ra_offset);
	if (stack_size) sp = (rt_uint32_t *)((rt_uint32_t)sp + stack_size);

	// repeat backwar scanning
	for (depth = 1; depth < size && ra && ((( rt_uint32_t )ra > KSEG0) && (( rt_uint32_t )ra < KSEG1)); ++depth)
	{
		rt_kprintf("RA[%2d] : [0x%08x]\n", depth ,ra);
		{
			extern void rt_thread_exit(void);
			if ((rt_uint32_t)ra == (rt_uint32_t)(&rt_thread_exit))
				return depth;
			else if(((rt_uint32_t)ra < rtboot_data.relocation_base) || ((rt_uint32_t)ra > (rtboot_data.relocation_base + rtboot_data.rtboot_length)))
				return depth;
		}
		
		ra_offset = 0;
		stack_size = 0;

		for ( addr = ra; !ra_offset || !stack_size; -- addr )
		{
			switch( *addr & INST_OP_MASK)
			{
				case ADDUI_SP_INST:
					stack_size = abs((short)(*addr&INST_OFFSET_MASK));
					break;

				case SW_RA_INST:
					ra_offset = abs((short)(*addr&INST_OFFSET_MASK));
					break;

				case 0x3c1c0000:
					rt_kprintf("Found raw fpic flag,end of trace\n");
					return depth +1;

				default:
					break;
			}
		}

		ra = *(rt_uint32_t **)((rt_uint32_t)sp + ra_offset);
		sp = (rt_uint32_t *)((rt_uint32_t)sp + stack_size);
	}

	return depth;
}

rt_int32_t calltrace(void)
{
	rt_uint32_t *addr;
	rt_uint32_t *ra;
	rt_uint32_t *sp;
	rt_uint32_t *pc;
	rt_int32_t size = 8, depth;

	rt_size_t ra_offset;
	rt_size_t stack_size;

	// get current $a and $sp
	__asm__ __volatile__ (
			" move %0, $ra\n"
			" move %1, $sp\n"
			" move %2, $t9\n"
			: "=r"(ra), "=r"(sp), "=r"(pc)
			);
	
	rt_kprintf("Calltrace:[0x%08x]\n", (rt_uint32_t)pc);

	// scanning to find the size of hte current stack frame
	stack_size  = 0;

	for ( addr = (rt_uint32_t *)&calltrace; !stack_size; ++addr)
	{
		if ((*addr & INST_OP_MASK ) == ADDUI_SP_INST )
			stack_size = abs((short)(*addr&INST_OFFSET_MASK));
		else if ( *addr == JR_RA_INST )
			break;
	}

	sp = (rt_uint32_t *) (( rt_uint32_t )sp + stack_size);

	// repeat backwar scanning
	for ( depth = 0; depth < size && ra  && ((( rt_uint32_t )ra > KSEG0) && (( rt_uint32_t )ra < KSEG1)); ++ depth )
	{
		rt_kprintf("RA[%2d] : [0x%08x]\n", depth, ra);
		{
			extern void rt_thread_exit(void);
			if ((rt_uint32_t)ra == (rt_uint32_t)(&rt_thread_exit))
				return depth;
			else if(((rt_uint32_t)ra < rtboot_data.relocation_base) || ((rt_uint32_t)ra > (rtboot_data.relocation_base + rtboot_data.rtboot_length)))
				return depth;
		}

		ra_offset = 0;
		stack_size = 0;

		for ( addr = ra; !ra_offset || !stack_size; -- addr )
		{
			switch( *addr & INST_OP_MASK)
			{
				case ADDUI_SP_INST:
					stack_size = abs((short)(*addr&INST_OFFSET_MASK));
					break;

				case SW_RA_INST:
					ra_offset = (short)(*addr&INST_OFFSET_MASK);
					break;

				case 0x3c1c0000:
					rt_kprintf("Found raw fpic flag,end of trace\n");
					return depth +1;

				default:
					break;
			}
		}

		ra = *(rt_uint32_t **)((rt_uint32_t)sp + ra_offset);
		sp = (rt_uint32_t*) ((rt_uint32_t)sp+stack_size );
	}

	return depth;
}

FINSH_FUNCTION_EXPORT(calltrace, print calltrace);
MSH_CMD_EXPORT(calltrace, print calltrace);
