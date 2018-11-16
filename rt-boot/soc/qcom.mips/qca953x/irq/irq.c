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

#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_reset.h>

static struct rt_irq_desc qca953x_irq_handle_table[32];

static void qca953x_interrupt_handler(int vector, void *param)
{
    rt_kprintf("Unhandled soc interrupt %d occured!!!\n", vector);
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void qca953x_interrupt_mask(int vector)
{
	if((vector < 0) || (vector > 7))
		return;
	qca_soc_reg_read_set(QCA_RST_MISC_INTERRUPT_MASK_REG,1<<vector);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void qca953x_interrupt_umask(int vector)
{
	if((vector < 0) || (vector > 31))
		return;
	qca_soc_reg_read_clear(QCA_RST_MISC_INTERRUPT_MASK_REG,1<<vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t qca953x_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if (vector >= 0 && vector < 32)
    {
        old_handler = qca953x_irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
        rt_strncpy(qca953x_irq_handle_table[vector].name, name, RT_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */
        qca953x_irq_handle_table[vector].handler = handler;
        qca953x_irq_handle_table[vector].param = param;
    }

    return old_handler;
}

static void qca953x_interrupt_dispatch(int vector,void * para)
{
	rt_uint32_t qca953x_irq_status;
	rt_uint32_t idx;
	rt_isr_handler_t irq_func;
    void *param;
	
	qca953x_irq_status = qca_soc_reg_read(QCA_RST_MISC_INTERRUPT_STATUS_REG) & qca_soc_reg_read(QCA_RST_MISC_INTERRUPT_MASK_REG);
	for (idx = 0; idx < 32; idx ++)
	{
		if(qca953x_irq_status & (1<<idx))
		{
			// 找到中断处理函数
    		irq_func = qca953x_irq_handle_table[idx].handler;
    		param    = qca953x_irq_handle_table[idx].param;

    		// 执行中断处理函数
    		irq_func(idx, param);
			
			#ifdef RT_USING_INTERRUPT_INFO
    		qca953x_irq_handle_table[idx].counter++;
			#endif
		}
	}
	qca_soc_reg_read_clear(QCA_RST_MISC_INTERRUPT_STATUS_REG,qca953x_irq_status);
}


/**
 * This function will initialize hardware interrupt
 */
void qca953x_interrupt_init(void)
{
	rt_uint32_t idx;
    rt_memset(qca953x_irq_handle_table, 0x00, sizeof(qca953x_irq_handle_table));
	
	for (idx = 0; idx < 32; idx ++)
    {
        qca953x_irq_handle_table[idx].handler = qca953x_interrupt_handler;
    }
	
	qca_soc_reg_write(QCA_RST_MISC_INTERRUPT_MASK_REG,0);
	qca_soc_reg_read_set(QCA_RST_MISC_INTERRUPT_STATUS_REG,0);
	
	rt_hw_interrupt_install(6, qca953x_interrupt_dispatch,0, "qca953x_irq_handler");
	rt_hw_interrupt_mask(6);
}

/*@}*/


