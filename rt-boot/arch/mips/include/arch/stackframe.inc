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
#ifndef __STACKFRAME_H__
#define __STACKFRAME_H__

#include <arch/mipsregs.h>

#define LONGSIZE	4
#define PTRSIZE     4

/*
 * Stack layout for the INT exception handler
 * Derived from the stack layout described in asm-mips/stackframe.h
 *
 * The first PTRSIZE*6 bytes are argument save space for C subroutines.
 */

//#define PT_R0		(PTRSIZE*6)			/* 0 */
#define PT_R0		(0)						/* 0 */
#define PT_R1		((PT_R0) + LONGSIZE)	/* 1 */
#define PT_R2		((PT_R1) + LONGSIZE)	/* 2 */
#define PT_R3		((PT_R2) + LONGSIZE)	/* 3 */
#define PT_R4		((PT_R3) + LONGSIZE)	/* 4 */
#define PT_R5		((PT_R4) + LONGSIZE)	/* 5 */
#define PT_R6		((PT_R5) + LONGSIZE)	/* 6 */
#define PT_R7		((PT_R6) + LONGSIZE)	/* 7 */
#define PT_R8		((PT_R7) + LONGSIZE)	/* 8 */
#define PT_R9		((PT_R8) + LONGSIZE)	/* 9 */
#define PT_R10		((PT_R9) + LONGSIZE)	/* 10 */
#define PT_R11		((PT_R10) + LONGSIZE)	/* 11 */
#define PT_R12		((PT_R11) + LONGSIZE)	/* 12 */
#define PT_R13		((PT_R12) + LONGSIZE)	/* 13 */
#define PT_R14		((PT_R13) + LONGSIZE)	/* 14 */
#define PT_R15		((PT_R14) + LONGSIZE)	/* 15 */
#define PT_R16		((PT_R15) + LONGSIZE)	/* 16 */
#define PT_R17		((PT_R16) + LONGSIZE)	/* 17 */
#define PT_R18		((PT_R17) + LONGSIZE)	/* 18 */
#define PT_R19		((PT_R18) + LONGSIZE)	/* 19 */
#define PT_R20		((PT_R19) + LONGSIZE)	/* 20 */
#define PT_R21		((PT_R20) + LONGSIZE)	/* 21 */
#define PT_R22		((PT_R21) + LONGSIZE)	/* 22 */
#define PT_R23		((PT_R22) + LONGSIZE)	/* 23 */
#define PT_R24		((PT_R23) + LONGSIZE)	/* 24 */
#define PT_R25		((PT_R24) + LONGSIZE)	/* 25 */
#define PT_R26		((PT_R25) + LONGSIZE)	/* 26 */
#define PT_R27		((PT_R26) + LONGSIZE)	/* 27 */
#define PT_R28		((PT_R27) + LONGSIZE)	/* 28 */
#define PT_R29		((PT_R28) + LONGSIZE)	/* 29 */
#define PT_R30		((PT_R29) + LONGSIZE)	/* 30 */
#define PT_R31		((PT_R30) + LONGSIZE)	/* 31 */

/*
 * Saved special registers
 */
#define PT_STATUS	((PT_R31) + LONGSIZE)	/* 32 */
#define PT_HI		((PT_STATUS) + LONGSIZE)	/* 33 */
#define PT_LO		((PT_HI) + LONGSIZE)	/* 34 */
#define PT_BADVADDR	((PT_LO) + LONGSIZE)	/* 35 */
#define PT_CAUSE	((PT_BADVADDR) + LONGSIZE)	/* 36 */
#define PT_EPC		((PT_CAUSE) + LONGSIZE)	/* 37 */

#define PT_SIZE		((((PT_EPC) + LONGSIZE) + (PTRSIZE-1)) & ~(PTRSIZE-1))

		.global mips_irq_mask

		.macro	SAVE_AT
		.set	push
		.set	noat
		sw		at, PT_R1(sp)
		.set	pop
		.endm

		.macro	SAVE_TEMP
		mfhi	v1
		sw		t0, PT_R8(sp)
		sw		t1, PT_R9(sp)
		sw		v1, PT_HI(sp)
		mflo	v1
		sw		t2, PT_R10(sp)
		sw		t3, PT_R11(sp)
		sw		v1,  PT_LO(sp)
		sw		t4, PT_R12(sp)
		sw		t5, PT_R13(sp)
		sw		t6, PT_R14(sp)
		sw		t7, PT_R15(sp)
		sw		t8, PT_R24(sp)
		.endm

		.macro	SAVE_STATIC
		sw		s0, PT_R16(sp)
		sw		s1, PT_R17(sp)
		sw		s2, PT_R18(sp)
		sw		s3, PT_R19(sp)
		sw		s4, PT_R20(sp)
		sw		s5, PT_R21(sp)
		sw		s6, PT_R22(sp)
		sw		s7, PT_R23(sp)
		sw		s8, PT_R30(sp)
		.endm

		.macro	SAVE_SOME
		.set	push
		.set	noat
		.set	reorder
        move	k1, sp
		subu	sp, k1, PT_SIZE
		sw		k1, PT_R29(sp)
		sw		v1, PT_R3(sp)
		sw		zero, PT_R0(sp)
		mfc0	v1, CP0_STATUS
		sw		v0, PT_R2(sp)
		sw		v1, PT_STATUS(sp)
		li      k1,0xFFFFFFE0
		and     v1, k1
		mtc0    v1, CP0_STATUS
		ehb
		sw		a0, PT_R4(sp)
		mfc0	v1, CP0_CAUSE
		sw		a1, PT_R5(sp)
		sw		v1, PT_CAUSE(sp)
		sw		a2, PT_R6(sp)
		mfc0	v1, CP0_EPC
		sw		a3, PT_R7(sp)
		sw		v1, PT_EPC(sp)
		sw		t9, PT_R25(sp)
		sw		gp, PT_R28(sp)
		sw		ra, PT_R31(sp)
		.set	pop
		.endm

		.macro	SAVE_ALL
		SAVE_SOME
		SAVE_AT
		SAVE_TEMP
		SAVE_STATIC
		.endm

		.macro	RESTORE_AT
		.set	push
		.set	noat
		lw		at,  PT_R1(sp)
		.set	pop
		.endm

		.macro	RESTORE_TEMP
		lw		t8, PT_LO(sp)
		lw		t0, PT_R8(sp)
		lw		t1, PT_R9(sp)
		mtlo	t8
		lw		t8, PT_HI(sp)
		lw		t2, PT_R10(sp)
		lw		t3, PT_R11(sp)
		mthi	t8
		lw		t4, PT_R12(sp)
		lw		t5, PT_R13(sp)
		lw		t6, PT_R14(sp)
		lw		t7, PT_R15(sp)
		lw		t8, PT_R24(sp)
		.endm

		.macro	RESTORE_STATIC
		lw		s0, PT_R16(sp)
		lw		s1, PT_R17(sp)
		lw		s2, PT_R18(sp)
		lw		s3, PT_R19(sp)
		lw		s4, PT_R20(sp)
		lw		s5, PT_R21(sp)
		lw		s6, PT_R22(sp)
		lw		s7, PT_R23(sp)
		lw		s8, PT_R30(sp)
		.endm

		.macro	RESTORE_SOME
		.set	push
		.set	reorder
		.set	noat
		lw		v0, PT_STATUS(sp)
		li      v1, 0xFFFF00F9
		and     v0, v1
		la		v1, mips_irq_mask
		lw		v1, 0(v1)
		or      v0, v1
		ori     v0, 3
		mtc0	v0, CP0_STATUS
		ehb
		lw		v1, PT_EPC(sp)
		mtc0	v1, CP0_EPC
		ehb
		lw		ra, PT_R31(sp)
		lw		gp, PT_R28(sp)
		lw		t9, PT_R25(sp)
		lw		a3,  PT_R7(sp)
		lw		a2,  PT_R6(sp)
		lw		a1,  PT_R5(sp)
		lw		a0,  PT_R4(sp)
		lw		v1,  PT_R3(sp)
		lw		v0,  PT_R2(sp)
		.set	pop
		.endm

		.macro	RESTORE_SP_AND_RET
		lw		sp, PT_R29(sp)
		eret
		.endm


		.macro	RESTORE_SP
		lw		sp, PT_R29(sp)
		.endm

		.macro	RESTORE_ALL
		RESTORE_TEMP
		RESTORE_STATIC
		RESTORE_AT
		RESTORE_SOME
		RESTORE_SP
		.endm

		.macro	RESTORE_ALL_AND_RET
		RESTORE_TEMP
		RESTORE_STATIC
		RESTORE_AT
		RESTORE_SOME
		RESTORE_SP_AND_RET
		.endm

#endif /* end of __STACKFRAME_H__ */

