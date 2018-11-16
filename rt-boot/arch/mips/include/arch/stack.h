/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 99, 2001 Ralf Baechle
 * Copyright (C) 1994, 1995, 1996 Paul M. Antoine.
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2007  Maciej W. Rozycki
 */
#ifndef __STACK_H__
#define __STACK_H__

typedef struct _arch_stack_frame{
	rt_uint32_t zero;
	rt_uint32_t at;
	rt_uint32_t v0;
	rt_uint32_t v1;
	rt_uint32_t a0;
	rt_uint32_t a1;
	rt_uint32_t a2;
	rt_uint32_t a3;
	rt_uint32_t t0;
	rt_uint32_t t1;
	rt_uint32_t t2;
	rt_uint32_t t3;
	rt_uint32_t t4;
	rt_uint32_t t5;
	rt_uint32_t t6;
	rt_uint32_t t7;
	rt_uint32_t s0;
	rt_uint32_t s1;
	rt_uint32_t s2;
	rt_uint32_t s3;
	rt_uint32_t s4;
	rt_uint32_t s5;
	rt_uint32_t s6;
	rt_uint32_t s7;
	rt_uint32_t t8;
	rt_uint32_t t9;
	rt_uint32_t k0;
	rt_uint32_t k1;
	rt_uint32_t gp;
	rt_uint32_t sp;
	rt_uint32_t s8;
	rt_uint32_t ra;
	rt_uint32_t status;
	rt_uint32_t hi;
	rt_uint32_t lo;
	rt_uint32_t badvaddr;
	rt_uint32_t cause;
	rt_uint32_t pc;
} arch_stack_frame;

void arch_stack_dump(rt_uint32_t sp);

#endif /* end of __STACKFRAME_H__ */

