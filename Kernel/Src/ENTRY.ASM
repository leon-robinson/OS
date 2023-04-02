
SECTION .kernel_entry

GLOBAL Init
EXTERN Main

Init:
	CALL Main
	JMP $