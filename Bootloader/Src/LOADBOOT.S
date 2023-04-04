[ORG 0xDE00]
[BITS 16]

LoadBootloader:
	MOV SI, ReadPacket
	MOV AH, 42H
	INT 13H
	JNC .OK
.E:
	MOV AH, 0xE
	MOV AL, 'B'
	INT 10H
	CLI
	HLT
	JMP $
.OK:
	JMP 7C00H

ReadPacket:
	DB 10H
	DB 0
	DW (12 * 4)
	DW 7C00H
	DW 0
	DD (11000H / 512)
	DD 0

TIMES 200H-($-$$) DB 0