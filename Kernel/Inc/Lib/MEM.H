#pragma once

#include <TYPE.H>

void Memcpy(void *dest, void *src, u64 count);
void Memmove(void *dest, void *src, u64 count);
void Memset(void *dest, u8 val, u64 count);
void MemsetU32(void *dest, u32 val, u64 count);
u64 Memcmp(void *src1, void *src2, u64 size);
u32 Strlen(const char *str);
char *Reverse(char *str, u32 i, u32 j);
char *ItoaU64(u64 val, char *str, int base);
