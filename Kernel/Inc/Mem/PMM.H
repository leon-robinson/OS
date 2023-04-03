#pragma once

#include <TYPE.H>
#include <BOOT.H>

#define PAGE_SIZE 0x1000
extern u64 mem_top;

u64 GetTotalMem();
u64 GetUsableMem();
u64 GetUsedMem();
u64 GetFreeMem();

void PMMInit(struct MemoryInfo *mem_info, u64 kernel_size, u64 kernel_start);
void *AllocPage(u32 count);
void FreePage(void *ptr, u32 count);
