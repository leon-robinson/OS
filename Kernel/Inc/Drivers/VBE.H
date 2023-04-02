#pragma once

#include <TYPE.H>
#include <BOOT.H>

#define DEFAULT_COLOR 0x0984e8

void VBEInit(struct VBEInfo *vbe_info);
void VBEUpdate();
u16 GetWidth();
u16 GetHeight();
u16 GetPitch();
u32 GetBytesPP();
u32 *GetFirstBuffer();
u64 GetSize();
