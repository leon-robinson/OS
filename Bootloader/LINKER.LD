OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(Init)

SECTIONS {
	. = 0x7c00;

	.cd_entry : {
		*(.cd.entry .cd_entry.*)
	}

	.text : {
		*(.text .text.*)
	}

	.rodata : {
		*(.rodata .rodata.*)
	}

	.data : {
		*(.data .data.*)
	}

	.bss : {
		*(COMMON)
		*(.bss .bss.*)
	}

	/DISCARD/ : {
		*(.eh_frame)
		*(.note .note.*)
	}
}