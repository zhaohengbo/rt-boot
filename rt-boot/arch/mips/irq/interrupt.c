/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date                  Author       Notes
 * 2010-10-15     Bernard      first version
 * 2010-10-15     lgnq           modified for LS1B
 * 2013-03-29     aozima       Modify the interrupt interface implementations.
 * 2015-07-06     chinesebear modified for loongson 1c
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>

#include <arch/mipsregs.h>
#include <arch/stack.h>
#include <arch/calltrace.h>

extern rt_uint32_t rt_interrupt_nest;
rt_uint32_t rt_system_stack;
rt_uint32_t rt_interrupt_from_thread;
rt_uint32_t rt_interrupt_to_thread;
rt_uint32_t rt_thread_switch_interrupt_flag;

static struct rt_irq_desc irq_handle_table[8];
/*static*/ rt_uint32_t rt_hw_system_stack[0x1000];

rt_uint32_t mips_irq_mask;

/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

static void rt_hw_syscall_handler(int vector, void *param)
{
    rt_kprintf("Unhandled syscall %d occured!!!\n", vector);
}

static void rt_hw_interrupt_handler(int vector, void *param)
{
    rt_kprintf("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void rt_hw_interrupt_init(void)
{
    rt_int32_t idx;

    rt_memset(irq_handle_table, 0x00, sizeof(irq_handle_table));
	
	for (idx = 0; idx < 2; idx ++)
    {
        irq_handle_table[idx].handler = rt_hw_syscall_handler;
    }
	
    for (idx = 2; idx < 8; idx ++)
    {
        irq_handle_table[idx].handler = rt_hw_interrupt_handler;
    }

    /* init interrupt nest, and context in thread sp */
    rt_interrupt_nest = 0;
    rt_interrupt_from_thread = 0;
    rt_interrupt_to_thread = 0;
    rt_thread_switch_interrupt_flag = 0;
	mips_irq_mask = 0;
	rt_system_stack = (rt_uint32_t)&(rt_hw_system_stack) + sizeof(rt_hw_system_stack) - 4;
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_mask(int vector)
{
    /* mask interrupt */
    volatile rt_uint32_t c0_status;
	rt_base_t level;
	
	if((vector < 0) || (vector > 7))
		return;
	level = rt_hw_interrupt_disable();
	c0_status = read_c0_status();
	mips_irq_mask |= 1<<(vector + 8);
	mips_irq_mask &= 0x0000FF00;
	c0_status &= 0xFFFF00FF;
	c0_status |= mips_irq_mask;
	write_c0_status(c0_status);
	rt_hw_interrupt_enable(level);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_umask(int vector)
{
	volatile rt_uint32_t c0_status;
	rt_base_t level;
	
	if((vector < 0) || (vector > 7))
		return;
	level = rt_hw_interrupt_disable();
	c0_status = read_c0_status();
	mips_irq_mask &= ~(1<<(vector + 8));
	mips_irq_mask &= 0x0000FF00;
	c0_status &= 0xFFFF00FF;
	c0_status |= mips_irq_mask;
	write_c0_status(c0_status);
	rt_hw_interrupt_enable(level);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;
	rt_base_t level;

    if (vector >= 0 && vector < 8)
    {
		level = rt_hw_interrupt_disable();
        old_handler = irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
        rt_strncpy(irq_handle_table[vector].name, name, RT_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
        irq_handle_table[vector].handler = handler;
        irq_handle_table[vector].param = param;
		rt_hw_interrupt_enable(level);
    }

    return old_handler;
}


/**
 * 执行中断处理函数
 * @number 中断号
 */
void mips_do_interrupt(rt_uint32_t sp,int number)
{
    rt_isr_handler_t irq_func;
    void *param;

    // 找到中断处理函数
    irq_func = irq_handle_table[number].handler;
    param    = irq_handle_table[number].param;

    // 执行中断处理函数
    irq_func(number, param);
    
#ifdef RT_USING_INTERRUPT_INFO
    irq_handle_table[number].counter++;
#endif
}

static const char *cause_strings[32] =
{
  /*  0 */ "Int",
  /*  1 */ "TLB Mods",
  /*  2 */ "TLB Load",
  /*  3 */ "TLB Store",
  /*  4 */ "Address Load",
  /*  5 */ "Address Store",
  /*  6 */ "Instruction Bus Error",
  /*  7 */ "Data Bus Error",
  /*  8 */ "Syscall",
  /*  9 */ "Breakpoint",
  /* 10 */ "Reserved Instruction",
  /* 11 */ "Coprocessor Unuseable",
  /* 12 */ "Overflow",
  /* 13 */ "Trap",
  /* 14 */ "Instruction Virtual Coherency Error",
  /* 15 */ "FP Exception",
  /* 16 */ "Reserved 16",
  /* 17 */ "Reserved 17",
  /* 18 */ "Reserved 18",
  /* 19 */ "Reserved 19",
  /* 20 */ "Reserved 20",
  /* 21 */ "Reserved 21",
  /* 22 */ "Reserved 22",
  /* 23 */ "Watch",
  /* 24 */ "Reserved 24",
  /* 25 */ "Reserved 25",
  /* 26 */ "Reserved 26",
  /* 27 */ "Reserved 27",
  /* 28 */ "Reserved 28",
  /* 29 */ "Reserved 29",
  /* 30 */ "Reserved 30",
  /* 31 */ "Data Virtual Coherency Error"
};

void mips_do_excpetion(rt_uint32_t sp,int code)
{
	rt_uint32_t irq_nest = rt_interrupt_get_nest();
	rt_kprintf("Mips exception 0x%08x:%s\n",code,cause_strings[code]);
	rt_kprintf("Current irq_nest: %d\n",irq_nest);
	arch_stack_dump(sp);
	calltrace_irq(sp);
    rt_kprintf("We can't handle excpetion now , so system halt!!!\n");
	while(1);
}

void rt_interrupt_dispatch(rt_uint32_t sp)
{
    volatile rt_uint32_t c0_status;
    volatile rt_uint32_t c0_cause;
    volatile rt_uint32_t cause_im;
    volatile rt_uint32_t status_im;
    volatile rt_uint32_t pending_im;
	volatile rt_uint32_t c0_ecptcode;

    /* check os timer */
    c0_status = read_c0_status();
    c0_cause = read_c0_cause();
	c0_ecptcode = (c0_cause&0x0000007C) >> 2;

    cause_im = c0_cause & ST0_IM;
    status_im = c0_status & ST0_IM;
    pending_im = cause_im & status_im;
	
	if(c0_ecptcode)
	{
		mips_do_excpetion(sp,c0_ecptcode);
	}
	else
	{
		if (pending_im & CAUSEF_IP0)
    	{
        	mips_do_interrupt(sp,0);
    	}
		
		if (pending_im & CAUSEF_IP1)
    	{
        	mips_do_interrupt(sp,1);
    	}
		
		if (pending_im & CAUSEF_IP2)
    	{
        	mips_do_interrupt(sp,2);
    	}
		
		if (pending_im & CAUSEF_IP3)
    	{
        	mips_do_interrupt(sp,3);
    	}
		
		if (pending_im & CAUSEF_IP4)
    	{
        	mips_do_interrupt(sp,4);
    	}
		
		if (pending_im & CAUSEF_IP5)
    	{
        	mips_do_interrupt(sp,5);
    	}
		
		if (pending_im & CAUSEF_IP6)
    	{
        	mips_do_interrupt(sp,6);
    	}
		
		if (pending_im & CAUSEF_IP7)
    	{
        	mips_do_interrupt(sp,7);
    	}
	}
}

/*@}*/


