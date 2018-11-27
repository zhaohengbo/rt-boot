OUTPUT_FORMAT("elf32-tradbigmips", "elf32-tradbigmips", "elf32-tradbigmips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text       :
	{
	  *(.text*)
	}

	. = ALIGN(4);
	.rodata  : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data  : { *(.data*) }

	. = .;
	_gp = ALIGN(16);

	.got  : {
	__got_start = .;
		*(.got)
	__got_end = .;
	}

	. = ALIGN(4);
	.sdata  : { *(.sdata*) }

	. = ALIGN(4);
	__rtobject_start = .;
	rtobject : { *(RTObject) }
	__rtobject_end = .;

	. = ALIGN(4);
	__fsymtab_start = .;
	fsymtab : { *(FSymTab) }
	__fsymtab_end = .;
	. = ALIGN(4);
	__vsymtab_start = .;
	vsymtab : { *(VSymTab) }
	__vsymtab_end = .;
	. = ALIGN(4);
	
	. = ALIGN(4);
	__lwip_tcp_pcb_list_start = .;
	lwip_tcp_pcb_list : { *(LWIPTCPPcbList) }
	__lwip_tcp_pcb_list_end = .;
	
	. = ALIGN(4);
	__lwip_tcp_state_str_start = .;
	lwip_tcp_state_str : { *(LWIPTCPStateStr) }
	__lwip_tcp_state_str_end = .;
	
	. = ALIGN(4);
	__lwip_mempool_start = .;
	lwip_mempool : { *(LWIPMemPool) }
	__lwip_mempool_end = .;
	
	. = ALIGN(4);
	__lwip_mempool_desc_start = .;
	lwip_mempool : { *(LWIPMemPoolDesc) }
	__lwip_mempool_desc_end = .;
	
	. = ALIGN(4);
	__lwip_cyclic_timer_start = .;
	lwip_cyclic_timer : { *(LWIPCyclicTimer) }
	__lwip_cyclic_timer_end = .;
	
	. = ALIGN(4);
	__lwip_err_str_start = .;
	lwip_err_str : { *(LWIPErrStr) }
	__lwip_err_str_end = .;
	
	. = ALIGN(4);
	__lwip_altcp_tcp_function_start = .;
	lwip_altcp_tcp_function : { *(LWIPAltcpTcpFunction) }
	__lwip_altcp_tcp_function_end = .;
	
	. = ALIGN(4);
	rtboot_data_end = .;

	. = ALIGN(4);
	.sbss  : { *(.sbss*) }
	.bss  : { *(.bss*) . = ALIGN(4); }
	rtboot_end = .;
}
