OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(Init)

SECTIONS {
	. = 0xffffffff80000000;

	.kernel_entry : {
		*(.kernel_entry .kernel_entry.*)
	}

	.text : {
		*(.text .text.*)
	}

	. += CONSTANT(MAXPAGESIZE);

	.rodata : {
		*(.rodata .rodata.*)
	}

	. += CONSTANT(MAXPAGESIZE);

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