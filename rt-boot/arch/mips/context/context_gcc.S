/*
 * File      : context_gcc.S
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-05-17     swkyer       first version
 * 2010-09-11     bernard      port to Loongson SoC3210
 * 2011-08-08     lgnq         port to Loongson LS1B
 */

#include <arch/mips.inc>
#include <arch/stackframe.inc>

    .text
    .set noreorder
/*
 * rt_base_t rt_hw_interrupt_disable()
 */
    .globl rt_hw_interrupt_disable
rt_hw_interrupt_disable:
    mfc0    v0, CP0_STATUS
    and     v1, v0, 0xfffffffe
    mtc0    v1, CP0_STATUS
	ehb
    jr      ra
    and		v0, 1

/*
 * void rt_hw_interrupt_enable(rt_base_t level)
 */
    .globl rt_hw_interrupt_enable
rt_hw_interrupt_enable:
	mfc0    v0, CP0_STATUS
	and		a0, 1
	or		v0, a0
    mtc0    v0, CP0_STATUS
	ehb
    jr      ra
    nop

/*
 * void rt_hw_context_switch(rt_uint32 from, rt_uint32 to)
 * a0 --> from
 * a1 --> to
 */
    .globl rt_hw_context_switch
rt_hw_context_switch:
    mtc0    ra, CP0_EPC
	ehb
    SAVE_ALL

    sw      sp, 0(a0)       /* store sp in preempted tasks TCB */
    lw      sp, 0(a1)       /* get new task stack pointer */

    RESTORE_ALL_AND_RET

/*
 * void rt_hw_context_switch_to(rt_uint32 to)/*
 * a0 --> to
 */
    .globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    lw      sp, 0(a0)       /* get new task stack pointer */

    RESTORE_ALL_AND_RET

/*
 * void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to)/*
 */
    .globl rt_thread_switch_interrupt_flag
    .globl rt_interrupt_from_thread
    .globl rt_interrupt_to_thread
    .globl rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    la      t0, rt_thread_switch_interrupt_flag
    lw      t1, 0(t0)
    nop
    bnez    t1, _reswitch
    nop
    li      t1, 0x01                       /* set rt_thread_switch_interrupt_flag to 1 */
    sw      t1, 0(t0)
    la      t0, rt_interrupt_from_thread   /* set rt_interrupt_from_thread */
    sw      a0, 0(t0)
_reswitch:
    la      t0, rt_interrupt_to_thread     /* set rt_interrupt_to_thread */
    sw      a1, 0(t0)
    jr      ra
    nop

/*
 * void rt_hw_context_switch_interrupt_do(rt_base_t flag)
 */
    .globl rt_interrupt_enter
    .globl rt_interrupt_leave
	.globl rt_interrupt_dispatch
	.globl rt_interrupt_get_nest
	.globl rt_system_stack
    .globl mips_irq_handle
	.globl mips_ecpt_handle
	.globl mips_tlb_handle
	.globl mips_cache_handle
mips_irq_handle:
    SAVE_ALL	
	move    gp, k0
	
	/* let s0 keep the current context sp */
	move    s0, sp
	
	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,irq_handle_no_change_sp
	nop
	
    /* switch to kernel stack */
	la      s1, rt_system_stack
	lw      s1, 0(s1)
	
    move    sp, s1
irq_handle_no_change_sp:
	
	la      t9,rt_interrupt_enter
    jalr    t9
    nop
	
	move    a0, s0 
	
	la      t9,rt_interrupt_dispatch
    jalr    t9
    nop
	la      t9,rt_interrupt_leave
    jalr    t9
    nop

	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,exit_irq_handle
	nop
	
    /* switch sp back to thread's context */
    move    sp, s0
    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and don't return
     */
    la      s0, rt_thread_switch_interrupt_flag
    lw      s1, 0(s0)
    beqz    s1, exit_irq_handle
    nop
    sw      zero, 0(s0)                     /* clear flag */
	nop

    /*
     * switch to the new thread
     */
    la      s0, rt_interrupt_from_thread
    lw      s1, 0(s0)
    nop
    sw      sp, 0(s1)                       /* store sp in preempted tasks's TCB */

    la      s0, rt_interrupt_to_thread
    lw      s1, 0(s0)
    nop
    lw      sp, 0(s1)                       /* get new task's stack pointer */

exit_irq_handle:
    RESTORE_ALL_AND_RET
	
mips_ecpt_handle:
    SAVE_ALL		
	move    gp, k0
	
	/* let s0 keep the current context sp */
    move    s0, sp
	
	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,ecpt_handle_no_change_sp
	nop
	
    /* switch to kernel stack */
	la      s1, rt_system_stack
	lw      s1, 0(s1)
	
    move    sp, s1
ecpt_handle_no_change_sp:
	
	la      t9,rt_interrupt_enter
    jalr    t9
    nop
	
	move    a0, s0 
	
	la      t9,rt_interrupt_dispatch
    jalr    t9
    nop
	la      t9,rt_interrupt_leave
    jalr    t9
    nop

	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,exit_ecpt_handle
	nop
	
    /* switch sp back to thread's context */
    move    sp, s0
    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and don't return
     */
    la      s0, rt_thread_switch_interrupt_flag
    lw      s1, 0(s0)
    beqz    s1, exit_ecpt_handle
    nop
    sw      zero, 0(s0)                     /* clear flag */
	nop

    /*
     * switch to the new thread
     */
    la      s0, rt_interrupt_from_thread
    lw      s1, 0(s0)
    nop
    sw      sp, 0(s1)                       /* store sp in preempted tasks's TCB */

    la      s0, rt_interrupt_to_thread
    lw      s1, 0(s0)
    nop
    lw      sp, 0(s1)                       /* get new task's stack pointer */

exit_ecpt_handle:
    RESTORE_ALL_AND_RET

mips_tlb_handle:
    SAVE_ALL	
	move    gp, k0
	
	/* let s0 keep the current context sp */
    move    s0, sp
	
	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,tlb_handle_no_change_sp
	nop
	
    /* switch to kernel stack */
	la      s1, rt_system_stack
	lw      s1, 0(s1)
	
    move    sp, s1
tlb_handle_no_change_sp:
	
	la      t9,rt_interrupt_enter
    jalr    t9
    nop
	
	move    a0, s0 
	
	la      t9,rt_interrupt_dispatch
    jalr    t9
    nop
	la      t9,rt_interrupt_leave
    jalr    t9
    nop

	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,exit_tlb_handle
	nop
	
    /* switch sp back to thread's context */
    move    sp, s0
    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and don't return
     */
    la      s0, rt_thread_switch_interrupt_flag
    lw      s1, 0(s0)
    beqz    s1, exit_tlb_handle
    nop
    sw      zero, 0(s0)                     /* clear flag */
	nop

    /*
     * switch to the new thread
     */
    la      s0, rt_interrupt_from_thread
    lw      s1, 0(s0)
    nop
    sw      sp, 0(s1)                       /* store sp in preempted tasks's TCB */

    la      s0, rt_interrupt_to_thread
    lw      s1, 0(s0)
    nop
    lw      sp, 0(s1)                       /* get new task's stack pointer */

exit_tlb_handle:
    RESTORE_ALL_AND_RET

mips_cache_handle:

	mfc0    k1, CP0_CONFIG
	ori     k1, 7
	subu    k1, 7
	ori     k1, 2
	mtc0    k1, CP0_CONFIG
	ehb

    SAVE_ALL	
	move    gp, k0
	
	/* let s0 keep the current context sp */
    move    s0, sp
	
	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,cache_handle_no_change_sp
	nop
	
    /* switch to kernel stack */
	la      s1, rt_system_stack
	lw      s1, 0(s1)
	
    move    sp, s1
cache_handle_no_change_sp:
	
	la      t9,rt_interrupt_enter
    jalr    t9
    nop
	
	move    a0, s0 
	
	la      t9,rt_interrupt_dispatch
    jalr    t9
    nop
	la      t9,rt_interrupt_leave
    jalr    t9
    nop

	la      t9,rt_interrupt_get_nest
    jalr    t9
    nop
	
	bnez	v0,exit_cache_handle
	nop
	
    /* switch sp back to thread's context */
    move    sp, s0
    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and don't return
     */
    la      s0, rt_thread_switch_interrupt_flag
    lw      s1, 0(s0)
    beqz    s1, exit_cache_handle
    nop
    sw      zero, 0(s0)                     /* clear flag */
	nop

    /*
     * switch to the new thread
     */
    la      s0, rt_interrupt_from_thread
    lw      s1, 0(s0)
    nop
    sw      sp, 0(s1)                       /* store sp in preempted tasks's TCB */

    la      s0, rt_interrupt_to_thread
    lw      s1, 0(s0)
    nop
    lw      sp, 0(s1)                       /* get new task's stack pointer */

exit_cache_handle:
    RESTORE_ALL_AND_RET

    .set reorder
