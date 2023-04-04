#pragma once

#include <TYPE.H>

#define ALIGN_DOWN(p, align) (p & ~(align - 1))
#define ALIGN_UP(p, align) ((p + align - 1) & ~(align - 1))
#define DIV_ROUNDUP(n, a) (ALIGN_UP(n, a) / a)

u16 ClampU16(u16 val, u16 min, u16 max);
int Abs(int x);
