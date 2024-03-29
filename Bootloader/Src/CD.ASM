
[SECTION .cd_entry]

[BITS 16]

GLOBAL Init
Init:
	JMP SkipBootInfo
	NOP

TIMES 8-($-$$) DB 0
BootInfo:
	.PVD:      DD 0
	.BootLBA:  DD 0
	.Len:      DD 0
	.Checksum: DD 0
	.Reserved: TIMES 40 DB 0
TIMES 90-($-$$) DB 0

SkipBootInfo:
	CLI
	CLD
	JMP 0000H:InitializeCS

%macro  TryVideoMode 2+

	MOV AX, %1
	MOV BX, %2
	MOV CL, 32
	CALL VBESetMode
	JNC .VIDEO_DONE

%endmacro

InitializeCS:
	XOR SI, SI
	MOV DS, SI
	MOV ES, SI
	MOV SS, SI
	MOV SP, 7C00H
	STI

	CALL ClearScreen

	CMP DWORD[1000H], 1234ABCDH
	JNE .SKIP

	MOV BYTE[IS_HDD], 1

.SKIP:
	PUSH EDX ; Save boot drive ID.

	CALL DiskExtCheck
	JC Error.DiskExt

	CALL CPUIDCheck
	JC Error.NoCPUID

	CALL LongModeCheck
	JC Error.NoLongMode

	CALL EnableA20Line
	JC Error.A20Fail

	MOV EDI, 1000H
	CALL FetchMemInfo

	CALL EnterUnrealMode

	; Copy memory map up.
	CLD
	MOV ESI, 1000H
	MOV EDI, 20000H
	MOV ECX, 5000H
	A32 REP MOVSB

	POP EDX ; Restore boot drive ID.
	CALL GetFSBlocks ; <- ECX

	CMP BYTE[IS_HDD], 0
	JE .SKIP1

	CALL LoadFSHDD
	JMP .SKIP2
.SKIP1:
	CALL LoadFS
.SKIP2:

	TryVideoMode 1920,1080
	TryVideoMode 1360,768
	TryVideoMode 1280,1024
	TryVideoMode 1024,768
	TryVideoMode 640,480

	JMP Error.Video

.VIDEO_DONE:
	JMP EnterLongMode


; UTLIITY FUNCTIONS
; -------------------------------

ClearScreen:
	XOR AH, AH
	MOV AL, 3H
	INT 10H
	RET

; ES:BP -> string
; CX = string length
; BL = color attribute
PrintString:
	XOR BH, BH
	XOR DX, DX
	MOV AH, 13H
	MOV AL, 01B
	INT 10H
	RET

; CF raised if NOT DETECTED.
DiskExtCheck:
	CLC
	MOV AH, 41H
	MOV BX, 55AAH
	INT 13H
	JC .RAISE
	CMP BX, 0AA55H
	JE .SKIP
.RAISE:
	STC
.SKIP:
	RET

; CF raised if NOT DETECTED.
CPUIDCheck:
	CLC

	PUSHFD
	POP EAX

	MOV ECX, EAX
	XOR EAX, (1 << 21)

	PUSH EAX
	POPFD

	PUSHFD
	POP EAX

	PUSH ECX
	POPFD

	XOR EAX, ECX

	JNZ .SKIP

	STC
.SKIP:
	RET

; CF raised if NOT DETECTED.
LongModeCheck:
	CLC

	MOV EAX, 0x80000000
	CPUID

	CMP EAX, 0x80000001
	JB .RAISE

	MOV EAX, 0x80000001
	CPUID

	TEST EDX, (1 << 29)
	JNZ .SKIP
.RAISE:
	STC
.SKIP:
	RET

; CF raised if failed to open A20 line.
EnableA20Line:
	CLC

	IN AL, 92H
	TEST AL, 2
	JNZ .SKIP
	OR AL, 2
	AND AL, 0FEH
	OUT 92H, AL

	CALL TestA20Line
	JNC .SKIP

	STC
.SKIP:
	RET

; CF raised if A20 line is NOT open.
TestA20Line:
	CLC

	PUSHF
	PUSH DS
	PUSH ES
	PUSH DI
	PUSH SI
	CLI

	XOR AX, AX
	MOV ES, AX

	NOT AX
	MOV DS, AX

	MOV DI, 500H
	MOV SI, 510H

	MOV AL, BYTE[ES:DI]
	PUSH AX

	MOV AL, BYTE[DS:SI]
	PUSH AX

	MOV BYTE[ES:DI], 00H
	MOV BYTE[DS:SI], 0FFH

	CMP BYTE[ES:DI], 0FFH

	POP AX
	MOV BYTE[DS:SI], AL

	POP AX
	MOV BYTE[ES:DI], AL

	XOR AX, AX
	JNE .SKIP

	STC
.SKIP:
	POP SI
	POP DI
	POP ES
	POP DS
	POPF
	STI

	RET

EnterUnrealMode:
	CLI
	PUSH DS
	PUSH ES

	LGDT [UnrealGDT.PTR]

	MOV EAX, CR0
	OR AL, 1
	MOV CR0, EAX

	JMP $+2

	MOV SI, 08H
	MOV DS, SI
	MOV ES, SI

	AND AL, 0FEH
	MOV CR0, EAX

	POP ES
	POP DS

	STI
	RET
UnrealGDT:
	.PTR:
		DW .END - .START - 1
		DD .START
	.START:
		DQ 0
		DB 0FFH, 0FFH, 0, 0, 0, 10010010B, 11001111B, 0
	.END:

; DL = Drive ID
; Returns:
;   ECX = Amount of blocks in ISO FS.
GetFSBlocks:
	MOV WORD[ReadPacket.BLOCKS], 1
	MOV EAX, DWORD[BootInfo.PVD]

	CMP BYTE[IS_HDD], 0
	JE .SKIP

	MOV ECX, 4
	PUSH EDX
	MUL ECX
	POP EDX

.SKIP:
	MOV DWORD[ReadPacket.LBA], EAX
	MOV WORD[ReadPacket.OFFSET], 2000H
	MOV AH, 42H
	MOV SI, ReadPacket
	INT 13H
	JC Error.Read

	MOV ECX, DWORD[2050H]

	RET

; ECX = FS blocks
; DL = Drive ID
LoadFS:
	MOV WORD[ReadPacket.BLOCKS], 32
	MOV DWORD[ReadPacket.LBA], 0
	MOV WORD[ReadPacket.OFFSET], 0
	MOV WORD[ReadPacket.SEGMENT], 5000H

	MOV EDI, 0A00000H
.LOOP:
	CMP ECX, 32
	JNL .SKIP

	MOV WORD[ReadPacket.BLOCKS], CX
	MOV BX, 1
.SKIP:
	MOV AH, 42H
	MOV SI, ReadPacket
	INT 13H
	JC Error.Read

	PUSH ECX
	PUSH EDI
	MOV ESI, 50000H
	MOV ECX, (2048 * 32)
	A32 REP MOVSB
	POP EDI
	POP ECX

	CMP BX, 1
	JE .DONE

	SUB ECX, 32
	ADD DWORD[ReadPacket.LBA], 32
	ADD EDI, (2048 * 32)

	JMP .LOOP
.DONE:
	RET

; ECX = FS blocks
; DL = Drive ID
LoadFSHDD:
	PUSH EDX
	MOV EAX, 4
	MUL ECX
	MOV ECX, EAX
	POP EDX

	MOV WORD[ReadPacket.BLOCKS], 32
	MOV DWORD[ReadPacket.LBA], 0
	MOV WORD[ReadPacket.OFFSET], 0
	MOV WORD[ReadPacket.SEGMENT], 5000H

	MOV EDI, 0A00000H
.LOOP:
	CMP ECX, 32
	JNL .SKIP

	MOV WORD[ReadPacket.BLOCKS], CX
	MOV BX, 1
.SKIP:
	MOV AH, 42H
	MOV SI, ReadPacket
	INT 13H
	JC Error.Read

	PUSH ECX
	PUSH EDI
	MOV ESI, 50000H
	MOV ECX, (512 * 32)
	A32 REP MOVSB
	POP EDI
	POP ECX

	CMP BX, 1
	JE .DONE

	SUB ECX, 32
	ADD DWORD[ReadPacket.LBA], 32
	ADD EDI, (512 * 32)

	JMP .LOOP
.DONE:
	RET

; AX = Width
; BX = Height
; CL = Bits Per Pixel
;
; FLAGS = Carry clear on success
; VBEInfo structure filled with info.
VBESetMode:
	MOV [.WIDTH], AX
	MOV [.HEIGHT], BX
	MOV [.BPP], CL

	STI

	PUSH ES
	MOV AX, 4F00H
	MOV DI, VBEInfoBlock
	INT 10H
	POP ES

	CMP AX, 4FH
	JNE .ERROR

	MOV AX, WORD[VBEInfoBlock.VIDEO_MODES]
	MOV [.OFFSET], AX
	MOV AX, WORD[VBEInfoBlock.VIDEO_MODES+2]
	MOV [.SEGMENT], AX

	MOV AX, [.SEGMENT]
	MOV FS, AX
	MOV SI, [.OFFSET]

.FIND_MODE:
	MOV DX, [FS:SI]
	ADD SI, 2
	MOV [.OFFSET], SI
	MOV [.MODE], DX
	XOR AX, AX
	MOV FS, AX

	CMP WORD[.MODE], 0FFFFH
	JE .ERROR

	PUSH ES
	MOV AX, 4F01H
	MOV CX, [.MODE]
	MOV DI, ModeInfoBlock
	INT 10H
	POP ES

	CMP AX, 4FH
	JNE .ERROR

	MOV AX, [.WIDTH]
	CMP AX, [ModeInfoBlock.WIDTH]
	JNE .NEXT_MODE

	MOV AX, [.HEIGHT]
	CMP AX, [ModeInfoBlock.HEIGHT]
	JNE .NEXT_MODE

	MOV AL, [.BPP]
	CMP AL, [ModeInfoBlock.BPP]
	JNE .NEXT_MODE

	; FOUND CORRECT MODE.
	MOV AX, [.WIDTH]
	MOV WORD[VBEInfo.WIDTH], AX
	MOV AX, [.HEIGHT]
	MOV WORD[VBEInfo.HEIGHT], AX
	MOV EAX, [ModeInfoBlock.FRAMEBUFFER]
	MOV DWORD[VBEInfo.FRAMEBUFFER], EAX
	MOV AX, [ModeInfoBlock.PITCH]
	MOV WORD[VBEInfo.PITCH], AX
	XOR EAX, EAX
	MOV AL, [.BPP]
	MOV BYTE[VBEInfo.BPP], AL
	SHR EAX, 3
	MOV DWORD[VBEInfo.BYTESPP], EAX

	; SET MODE.
	PUSH ES
	MOV AX, 04F02H
	MOV BX, [.MODE]
	OR BX, 4000H
	XOR DI, DI
	INT 10H
	POP ES

	CMP AX, 4FH
	JNE .ERROR

	CLC
	RET

.NEXT_MODE:
	MOV AX, [.SEGMENT]
	MOV FS, AX
	MOV SI, [.OFFSET]
	JMP .FIND_MODE
.ERROR:
	STC
	RET

.WIDTH: DW 0
.HEIGHT: DW 0
.BPP: DB 0
.SEGMENT: DW 0
.OFFSET: DW 0
.MODE: DW 0

; EDI = address
FetchMemInfo:
	MOV DWORD[.InitAddr], EDI
	MOV EAX, 0E820H
	MOV EDX, 534D4150H
	MOV ECX, 20
	MOV DWORD[ES:EDI], 0
	ADD EDI, 8
	XOR EBX, EBX
	INT 15H
	JC Error.MemInfo
.LOOP:
	ADD EDI, 20
	MOV EAX, DWORD[ES:.InitAddr]
	INC DWORD[ES:EAX]
	TEST EBX, EBX
	JZ .DONE

	MOV EAX, 0E820H
	MOV EDX, 534D4150H
	MOV ECX, 20
	INT 15H
	JNC .LOOP
.DONE:
	RET

.InitAddr: DD 0

; ES:BP -> error string
; CX = error string length
Error:
	CALL ClearScreen
	MOV BL, 4H ; dark red.
	CALL PrintString
	CLI
	HLT
.DiskExt:
	MOV BP, DiskExtError
	MOV CX, DiskExtErrorLen
	JMP Error
.NoCPUID:
	MOV BP, NoCPUIDError
	MOV CX, NoCPUIDErrorLen
	JMP Error
.NoLongMode:
	MOV BP, NoLongModeError
	MOV CX, NoLongModeErrorLen
	JMP Error
.A20Fail:
	MOV BP, A20FailError
	MOV CX, A20FailErrorLen
	JMP Error
.Read:
	MOV BP, ReadError
	MOV CX, ReadErrorLen
	JMP Error
.Video:
	MOV BP, VideoError
	MOV CX, VideoErrorLen
	JMP Error
.MemInfo:
	MOV BP, MemInfoError
	MOV CX, MemInfoErrorLen
	JMP Error

%define PAGE_PRESENT (1 << 0)
%define PAGE_WRITE   (1 << 1)

EnterLongMode:
	MOV EDI, 1000H

	PUSH DI
	MOV ECX, 1000H
	XOR EAX, EAX
	CLD
	A32 REP STOSD
	POP DI

	LEA EAX, [ES:DI + 1000H]
	OR EAX, PAGE_PRESENT | PAGE_WRITE
	MOV [ES:DI], EAX

	LEA EAX, [ES:DI + 2000H]
	OR EAX, PAGE_PRESENT | PAGE_WRITE
	MOV [ES:DI + 1000H], EAX

	LEA EAX, [ES:DI + 3000H]
	OR EAX, PAGE_PRESENT | PAGE_WRITE
	MOV [ES:DI + 2000H], EAX

	PUSH DI
	LEA DI, [DI + 3000H]
	MOV EAX, PAGE_PRESENT | PAGE_WRITE

.LOOP:
	MOV [ES:DI], EAX
	ADD EAX, 1000H
	ADD DI, 8
	CMP EAX, 200000H
	JB .LOOP

	POP DI

	MOV AL, 0FFH
	OUT 0A1H, AL
	OUT 21H, AL

	TIMES 2 NOP

	CLI
	LIDT [LongIDT]

	MOV EAX, 10100000B
	MOV CR4, EAX

	MOV EDX, EDI
	MOV CR3, EDX

	MOV ECX, 0C0000080H
	RDMSR

	OR EAX, 100H
	WRMSR

	LGDT [LongGDT.PTR]

	MOV EBX, CR0
	OR EBX, 80000001H
	MOV CR0, EBX

	JMP 0008H:LongMode

ALIGN 4
LongIDT:
	DW 0
	DD 0

LongGDT:
	DQ 0000000000000000H
	DQ 00209A0000000000H
	DQ 0000920000000000H
ALIGN 4
	DW 0
.PTR:
	DW $ - LongGDT - 1
	DD LongGDT

DiskExtError: DB "ERROR: DISK EXTENSIONS NOT DETECTED."
DiskExtErrorLen: EQU $-DiskExtError
NoCPUIDError: DB "ERROR: CPUID SUPPORT NOT DETECTED."
NoCPUIDErrorLen: EQU $-NoCPUIDError
NoLongModeError: DB "ERROR: 64-BIT CPU REQUIRED."
NoLongModeErrorLen: EQU $-NoLongModeError
A20FailError: DB "ERROR: FAILED TO OPEN THE A20 LINE."
A20FailErrorLen: EQU $-A20FailError
ReadError: DB "ERROR: FAILED TO READ FROM BOOT DISK."
ReadErrorLen: EQU $-ReadError
VideoError: DB "ERROR: FAILED TO SET VIDEO MODE."
VideoErrorLen: EQU $-VideoError
MemInfoError: DB "ERROR: FAILED TO GET MEMORY INFO."
MemInfoErrorLen: EQU $-MemInfoError

ReadPacket:
	  DB 10H
	  DB 00H
.BLOCKS:  DW 0000H
.OFFSET:  DW 0000H
.SEGMENT: DW 0000H
.LBA:     DQ 0000000000000000H

VBEInfoBlock:
	.SIG: DB "VESA"
	.VER: DW 0
	.OEM: DD 0
	.CAPABILITIES: DD 0
	.VIDEO_MODES: DD 0
	.VIDEO_MEMORY: DW 0
	.SOFTWARE_REV: DW 0
	.VENDOR: DD 0
	.PRODUCT_NAME: DD 0
	.PRODUCT_REV: DD 0
	.RESERVED: TIMES 222 DB 0
	.OEM_DATA: TIMES 256 DB 0


ModeInfoBlock:
	TIMES 16 DB 0
	.PITCH: DW 0
	.WIDTH: DW 0
	.HEIGHT: DW 0
	TIMES 3 DB 0
	.BPP: DB 0
	TIMES 14 DB 0
	.FRAMEBUFFER: DD 0
	TIMES 7 DB 0
	TIMES 206 DB 0

VBEInfo:
	.WIDTH: DW 0
	.HEIGHT: DW 0
	.BPP: DB 0
	.BYTESPP: DD 0
	.FRAMEBUFFER: DD 0
	.PITCH: DD 0

[BITS 64]

PML4: DQ 10000H
p: DQ 0

;
; RDI -> Page of size 1000H bytes
AllocateTable:
	MOV RDI, [PML4]
	ADD RDI, 4096

	PUSH RAX
	PUSH RBX
	MOV RAX, [p]
	MOV RBX, 4096
	MUL RBX ; RAX = p * 4096
	ADD RDI, RAX ; RDI = (PML4 + 4096) + (p * 4096)
	POP RBX
	POP RAX

	PUSH RAX
	PUSH DI
	MOV RCX, 1000H
	XOR RAX, RAX
	CLD
	REP STOSD
	POP DI
	POP RAX

	INC QWORD[p]

	RET

; RBX = Directory
; RCX = Entry
;
; RDI -> Result
Next:
	XOR RDI, RDI

	PUSH RAX
	PUSH RBX
	PUSH RCX
	PUSH RDX
	PUSH RSI

	MOV RAX, 8
	MUL RCX
	ADD RBX, RAX ; RBX = &Directory[Entry]
	MOV RSI, RBX ; RSI = &Directory[Entry]
	MOV RBX, QWORD[RBX] ; RBX = Directory[Entry]
	MOV RDX, RBX
	AND RDX, 1 ; RDX = Directory[Entry] & 1

	CMP RDX, 0
	JNE .NOTZERO ; if ((Directory[Entry] & 1) != 0)

.ZERO:
	CALL AllocateTable ; RDI = &FreePage
	PUSH RDI
	MOV RAX, 000FFFFFFFFFF000H
	AND RDI, RAX ; RDI = (p & 000FFFFFFFFFF000H)
	OR RDI, 11B ; RDI = (p & 000FFFFFFFFFF000H) | 11B
	MOV QWORD[RSI], RDI
	POP RDI
	JMP .SKIP
.NOTZERO:
	MOV RDI, RBX ; RDI = Directory[Entry]
	MOV RAX, 000FFFFFFFFFF000H
	AND RDI, RAX ; RDI = Directory[Entry] & 000FFFFFFFFFF000H
.SKIP:

	POP RSI
	POP RDX
	POP RCX
	POP RBX
	POP RAX

	RET

; RSI = Physical Address
; RDI = Virtual Address
; R12 = BOOL(HugePage)
; R13 = BOOL(WriteThrough)
MapPage:
	PUSH RSI
	PUSH RDX

	; PML4 Entry = R8
	MOV R8, RDI
	MOV RDX, (1FFH << 39)
	AND R8, RDX
	SHR R8, 39

	; PML3 Entry = R9
	MOV R9, RDI
	MOV RDX, (1FFH << 30)
	AND R9, RDX
	SHR R9, 30

	; PML2 Entry = R10
	MOV R10, RDI
	MOV RDX, (1FFH << 21)
	AND R10, RDX
	SHR R10, 21

	; PML1 Entry = R11
	MOV R11, RDI
	MOV RDX, (1FFH << 12)
	AND R11, RDX
	SHR R11, 12

	POP RDX

	PUSH RAX
	PUSH RDX

	; RAX = PML3
	MOV RBX, QWORD[PML4]
	MOV RCX, R8
	CALL Next
	MOV RAX, RDI

	; RDX = PML2
	MOV RBX, RAX
	MOV RCX, R9
	CALL Next
	MOV RDX, RDI

	CMP R12, 0
	JE .TYPICAL
.HUGE:
	MOV RAX, 8
	PUSH RDX
	MUL R10
	POP RDX
	ADD RDX, RAX
	OR RSI, 10000011B
	CMP R13, 0
	JE .C1
	OR RSI, 1000B
.C1:
	MOV QWORD[RDX], RSI
	JMP .SKIP
.TYPICAL:
	MOV RBX, RDX
	MOV RCX, R10
	CALL Next
	MOV RAX, 8
	MUL R11
	ADD RDI, RAX
	OR RSI, 11B
	CMP R13, 0
	JE .C0
	OR RSI, 1000B
.C0:
	MOV QWORD[RDI], RSI
.SKIP:
	POP RDX
	POP RAX

	POP RSI
	INVLPG [RSI]

	RET

MapFirst1GB:
	MOV RCX, 1000H
	XOR R12, R12
	XOR R13, R13

; Map 4096B-2MB.
.LOOP:
	PUSH RCX
	MOV RSI, RCX
	MOV RDI, RCX
	CALL MapPage
	POP RCX

	ADD RCX, 1000H

	CMP RCX, 200000H
	JNE .LOOP

	MOV RCX, 200000H
	MOV R12, 1
	XOR R13, R13

; Map 2MB-1GB.
.LOOP2:
	PUSH RCX
	MOV RSI, RCX
	MOV RDI, RCX
	CALL MapPage
	POP RCX

	ADD RCX, 200000H
	MOV RAX, (1024 * 1024 * 1024)
	CMP RCX, RAX
	JNE .LOOP2

	RET

MapKernel:
	MOV RCX, 400000H
	XOR R13, R13
	MOV R12, 1

.LOOP:
	PUSH RCX
	MOV RSI, RCX
	MOV RDI, RCX
	MOV RAX, (0FFFFFFFF80000000H - 400000H)
	ADD RDI, RAX
	CALL MapPage
	POP RCX

	ADD RCX, 200000H

	CMP RCX, 1000000H
	JNE .LOOP

	RET

MapFramebuffer:
	XOR RAX, RAX
	XOR RBX, RBX
	XOR RCX, RCX

	MOV AX, WORD[VBEInfo.WIDTH]
	MOV BX, WORD[VBEInfo.HEIGHT]
	MOV ECX, DWORD[VBEInfo.BYTESPP]

	MUL RBX
	MUL RCX

	MOV RDX, RAX ; RDX = Framebuffer size bytes.

	XOR R12, R12 ; 4K pages.
	MOV R13, 1
	XOR RCX, RCX
	MOV ECX, DWORD[VBEInfo.FRAMEBUFFER]

	ADD RDX, RCX ; RDX = end physical address.

	MOV RAX, 0FFFF8000C0000000H
	SUB RAX, RCX

.LOOP:
	PUSH RCX
	MOV RSI, RCX
	MOV RDI, RCX
	ADD RDI, RAX
	PUSH RDX
	PUSH RAX
	CALL MapPage
	POP RAX
	POP RDX
	POP RCX

	ADD RCX, 1000H

	CMP RCX, RDX
	JNE .LOOP

	RET

EXTERN LoadKernel

LongMode:
	MOV SI, 0010H
	MOV DS, SI
	MOV ES, SI
	MOV FS, SI
	MOV GS, SI
	MOV SS, SI

	MOV RDI, QWORD[PML4]
	XOR RAX, RAX
	MOV ECX, 1000H
	CLD
	REP STOSD

	CALL MapFirst1GB
	CALL MapKernel
	CALL MapFramebuffer

	MOV RDX, QWORD[PML4]
	MOV CR3, RDX

	CALL LoadKernel ; RAX = Kernel size

	MOV RDI, 0x6000 ; Boot Info Address, first argument for Kernel Main.
	PUSH RDI

	MOV QWORD[RDI], VBEInfo
	MOV QWORD[RDI+8], 20000H ; Memory map address.
	MOV QWORD[RDI+16], RAX ; Kernel size.
	MOV QWORD[RDI+24], 400000H ; Kernel start physical.

	POP RDI
	MOV RAX, 0xffffffff80000000
	JMP RAX

IS_HDD: DB 0
