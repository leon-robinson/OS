[BITS 16]
[ORG 0x7c00]

mov ah, 0xe
mov al, 'C'
int 0x10
jmp $

times 2048-($-$$) db 0
