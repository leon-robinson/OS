#pragma once

#include <TYPE.H>

struct VBEInfo {
	u16 width;
	u16 height;
	u8 bpp;
	u32 bytes_pp;
	u32 framebuffer;
	u32 pitch;
} __attribute__((packed));

struct BootInfo {
	struct VBEInfo *vbe_info;
} __attribute__((packed));
