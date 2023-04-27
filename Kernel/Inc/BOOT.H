#pragma once

#include <TYPE.H>

struct VBEInfo {
	u16 width;
	u16 height;
	u8 bpp;
	u32 bytes_pp;
	u32 framebuffer;
	u32 pitch;
} PACKED;

struct E820 {
	u64 address;
	u64 length;
	u32 type;
} PACKED;

struct MemoryInfo {
	u64 entry_count;
	struct E820 *entries;
} PACKED;

struct BootInfo {
	struct VBEInfo *vbe_info;
	struct MemoryInfo *memory_info;
	u64 kernel_size;
	u64 kernel_start;
} PACKED;
