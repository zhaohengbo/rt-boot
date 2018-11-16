/*
 * The second step boot code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <arch/mips.inc>
#include <arch/mipsregs.h>
#include <arch/cacheops.h>
#include <arch/addrspace.h>

	.globl system_early_init

	.set noreorder
	
	.globl _start
	.text
	.align 4
	
_start:
	//li		k0,0
	//beqz	k0,.
	//nop
	/* This maybe not so important in no-os bootloader, */
	/* but in rt-boot, we need a good boot enveriment */
	mtc0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI
	mtc0	zero, CP0_CAUSE
	li      k0, 0x10000004
	mtc0	k0, CP0_STATUS
	mtc0	zero, CP0_COUNT
	mtc0	zero, CP0_COMPARE
	/* As cache state is unknown, la t9 is allowed here */
	bal     mips_cache_reset
	nop
	/* Initialize GOT pointer. */
	bal		1f
	nop
.word _GLOBAL_OFFSET_TABLE_
1:
	lw		gp, 0(ra)
	
	li 		sp, 0x80001000//CFG_SDRAM_BASE + CFG_INIT_SP_OFFSET
	/* As we are in sencond step, DRAM is already initialized */
	la		t9, system_early_init
	jalr	t9
	nop
	
__halt:
	b		__halt
	nop
	
mips_cache_reset:
	mfc0    t0, CP0_CONFIG, 1
	ext     t5, t0, 0x16, 3
	ext     t6, t0, 0xA, 3
	li      t1, 2
	sllv    t5, t1, t5
	sllv    t6, t1, t6
	or      t7, t5, t6
	beqz	t7, _mips_cache_reset_exit
	nop
	beqz	t5, _mips_skip_icache_reset
	nop
	move    t4, t5
	/* determine cache size is complex, we can use max size */
	li      t2, MIPS_MAX_CACHE_SIZE //ICACHE_SIZE

    mtc0    zero, CP0_TAGLO
	mtc0	zero, $29	# C0_TagHi

	li      t0, K0BASE
	add     t2, t2, t0

_arai_icache_loop:
	cache   Index_Store_Tag_I, 0(t0)
	add     t0, t0, t4
	bne     t0, t2, _arai_icache_loop
	nop
	
_mips_skip_icache_reset:

	beqz	t6, _mips_skip_dcache_reset
	nop
	move    t4, t6
	li      t3, MIPS_MAX_CACHE_SIZE //DCACHE_SIZE

    mtc0    zero, CP0_TAGLO
	mtc0	zero, $29	# C0_TagHi
	mtc0	zero, $28, 2	# C0_DTagLo
	mtc0	zero, $29, 2	# C0_DTagHi
	li      t0, K0BASE
	add     t3, t3, t0
_arai_dcache_loop:
	cache   Index_Store_Tag_D, 0(t0)
	add     t0, t0, t4
	bne     t0, t3, _arai_dcache_loop
	nop
_mips_skip_dcache_reset:
	/* We can enable cache now */
	li      k0, 0x00000003
	mtc0	k0, CP0_CONFIG
_mips_cache_reset_exit:
	jr      ra
	nop
	