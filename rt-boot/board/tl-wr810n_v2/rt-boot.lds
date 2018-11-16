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

	rtboot_data_end = .;

	. = ALIGN(4);
	.sbss  : { *(.sbss*) }
	.bss  : { *(.bss*) . = ALIGN(4); }
	rtboot_end = .;
}
