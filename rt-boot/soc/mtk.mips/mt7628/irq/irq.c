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

#include <soc/mt7628/mt7628_mmap.h>
#include <soc/mt7628/mt7628_irq.h>

static struct rt_irq_desc mt7628_irq_handle_table[32];

static void mt7628_interrupt_handler(int vector, void *param)
{
    rt_kprintf("Unhandled soc interrupt %d occured!!!\n", vector);
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void mt7628_interrupt_mask(int vector)
{
	rt_base_t level;
	
	if((vector < 0) || (vector > 7))
		return;
	level = rt_hw_interrupt_disable();
	__REG(RALINK_IRQMASK_REG) |= 1<<vector;
	rt_hw_interrupt_enable(level);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void mt7628_interrupt_umask(int vector)
{
	rt_base_t level;
	
	if((vector < 0) || (vector > 31))
		return;
	level = rt_hw_interrupt_disable();
	__REG(RALINK_IRQMASK_REG) &= ~(1<<vector);
	rt_hw_interrupt_enable(level);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t mt7628_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;
	rt_base_t level;

    if (vector >= 0 && vector < 32)
    {
		level = rt_hw_interrupt_disable();
        old_handler = mt7628_irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
        rt_strncpy(mt7628_irq_handle_table[vector].name, name, RT_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
        mt7628_irq_handle_table[vector].handler = handler;
        mt7628_irq_handle_table[vector].param = param;
		rt_hw_interrupt_enable(level);
    }

    return old_handler;
}

static void mt7628_interrupt_dispatch(int vector,void * para)
{
	rt_uint32_t mt7628_irq_status;
	rt_uint32_t idx;
	rt_isr_handler_t irq_func;
    void *param;
	
	mt7628_irq_status = __REG(RALINK_IRQSTAT_REG);
	
	for (idx = 0; idx < 32; idx ++)
	{
		if(mt7628_irq_status & (1<<idx))
		{
			// 找到中断处理函数
    		irq_func = mt7628_irq_handle_table[idx].handler;
    		param    = mt7628_irq_handle_table[idx].param;

    		// 执行中断处理函数
    		irq_func(idx, param);
			
			#ifdef RT_USING_INTERRUPT_INFO
    		mt7628_irq_handle_table[idx].counter++;
			#endif
		}
	}
	__REG(RALINK_IRQEND_REG) |= mt7628_irq_status;
}


/**
 * This function will initialize hardware interrupt
 */
void mt7628_interrupt_init(void)
{
	rt_uint32_t idx;
    rt_memset(mt7628_irq_handle_table, 0x00, sizeof(mt7628_irq_handle_table));
	
	for (idx = 0; idx < 32; idx ++)
    {
        mt7628_irq_handle_table[idx].handler = mt7628_interrupt_handler;
    }
	
	__REG(RALINK_IRQMASK_REG) = 0;
	__REG(RALINK_IRQSTAT_REG) = 0;
	
	rt_hw_interrupt_install(5, mt7628_interrupt_dispatch,0, "mt7628_irq_handler");
	rt_hw_interrupt_mask(5);
}

void mt7628_interrupt_deinit(void)
{
	__REG(RALINK_IRQMASK_REG) = 0;
}

/*@}*/


