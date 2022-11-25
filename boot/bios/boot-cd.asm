bits 16
org 0x7c00

jmp skip_bpb
nop

times 8-($-$$) db 0
boot_info:
	.pvd: dd 0
	.lba: dd 0
	.len: dd 0
	.checksum: dd 0
	.reserved: times 40 db 0

times 90-($-$$) db 0

skip_bpb:
	cli
	cld
	jmp 0:.init_cs
.init_cs:
	xor si, si
	mov ds, si
	mov es, si
	mov ss, si
	mov sp, 0x7c00
	sti

	mov ax, 0x3
	int 0x10

	push edx

	pushfd
	pop eax
	mov ecx, eax
	xor eax, (1<<21)
	push eax
	popfd
	pushfd
	pop eax
	push ecx
	popfd
	xor eax, ecx
	jz error.no_cpuid

	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb error.no_longmode
	mov eax, 0x80000001
	cpuid
	test edx, (1<<29)
	jz error.no_longmode

	pop edx

	mov ah, 0x41
	mov bx, 0x55aa
	int 0x13
	jc error.disk_ext
	cmp bx, 0xaa55
	jne error.disk_ext

	call test_a20
	jnc .skip_a20

	in al, 0x92
	test al, 2
	jnz .skip_a20
	or al, 2
	and al, 0xfe
	out 0x92, al

	call test_a20
	jc error.a20_fail

.skip_a20:

	cli
	push ds
	push es
	lgdt [unreal_gdt.ptr]
	mov eax, cr0
	or al, 1
	mov cr0, eax
	jmp $+2
	mov si, 0x8
	mov ds, si
	mov es, si
	and al, 0xfe
	mov cr0, eax
	pop es
	pop ds
	sti

	mov word[read_packet.blocks], 1
	mov eax, dword[boot_info.pvd]
	mov dword[read_packet.lba], eax
	mov word[read_packet.offset], 0x2000
	mov ah, 0x42
	mov si, read_packet
	int 0x13
	jc error.read

	mov ecx, dword[0x2050]

	pushad
	mov bp, reading_disk_msg
	mov cx, reading_disk_msg_len
	call print_str_bios
	popad

	mov word[read_packet.blocks], 32
	mov dword[read_packet.lba], 0
	mov word[read_packet.offset], 0
	mov word[read_packet.segment], 0x5000

	mov edi, 0xA00000
read_loop:
	cmp ecx, 32
	jnl .skip

	mov word[read_packet.blocks], cx
	mov bx, 1
.skip:
	mov ah, 0x42
	mov si, read_packet
	int 0x13
	jc error.read

	push ecx
	mov esi, 0x50000
	mov ecx, (2048 * 32)
	a32 rep movsb
	pop ecx

	cmp bx, 1
	je .done

	sub ecx, 32
	add dword[read_packet.lba], 32
	add edi, (2048 * 32)

	jmp read_loop
.done:
	mov ax, 0x3
	int 0x10

	mov eax, 0xb8000
	mov byte[ds:eax], 'Y'
	mov byte[ds:eax+1], 0xE

	jmp $

error:
	mov ax, 0x3
	int 0x10
	call print_str_bios
.done:
	cli
	hlt
	jmp .done
.disk_ext:
	mov bp, disk_ext_error
	mov cx, disk_ext_error_len
	jmp error
.read:
	mov bp, read_error
	mov cx, read_error_len
	jmp error
.no_cpuid:
	mov bp, no_cpuid_error
	mov cx, no_cpuid_error_len
	jmp error
.no_longmode:
	mov bp, no_longmode_error
	mov cx, no_longmode_error_len
	jmp error
.a20_fail:
	mov bp, a20_fail_error
	mov cx, a20_fail_error_len
	jmp error

; BP = string
; CX = len
print_str_bios:
	xor bx, bx
	mov ah, 0x13
	mov al, 0b1
	mov bl, 0xf
	xor dx, dx
	int 0x10
	ret

; Carry flag set if not enabled.
test_a20:
	pushf
	push ds
	push es
	push di
	push si
	cli

	xor ax, ax
	mov es, ax

	not ax
	mov ds, ax

	mov di, 0x500
	mov si, 0x510

	mov al, byte[es:di]
	push ax

	mov al, byte[ds:si]
	push ax

	mov byte[es:di], 0
	mov byte[ds:si], 0xff

	cmp byte[es:di], 0xff

	pop ax
	mov byte[ds:si], al

	pop ax
	mov byte[es:di], al

	mov ax, 0
	jne .done

	stc
.done:
	pop si
	pop di
	pop es
	pop ds
	popf
	ret

disk_ext_error: db "FAILED BOOT: Your PC does not support 64-bit disk extensions."
disk_ext_error_len: equ $-disk_ext_error
read_error: db "FAILED BOOT: Failed to read from boot drive."
read_error_len: equ $-read_error
no_cpuid_error: db "FAILED BOOT: Your PC does not support CPUID."
no_cpuid_error_len: equ $-no_cpuid_error
no_longmode_error: db "FAILED BOOT: Your PC does not support 64-bit long mode."
no_longmode_error_len: equ $-no_longmode_error
reading_disk_msg: db "Loading system from disk..."
reading_disk_msg_len: equ $-reading_disk_msg
a20_fail_error: db "FAILED BOOT: Failed to enable the A20 line."
a20_fail_error_len: equ $-a20_fail_error

read_packet:
	  db 0x10
	  db 0
.blocks:  dw 0
.offset:  dw 0
.segment: dw 0
.lba:     dq 0

unreal_gdt:
	.ptr:
		dw .end - .start - 1
		dd .start
	.start:
		dd 0, 0
		db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
	.end:

times 2048-($-$$) db 0
