#pragma once

#include <TYPE.H>

#define PS2_DATA 0x60
#define PS2_CMD 0x64

#define PS2_SET_DEFAULTS 0xF6
#define PS2_ENABLE_DATA_REPORTING 0xF4

void PS2MouseInit();
void MouseOnInterrupt();
u16 GetMouseX();
u16 GetMouseY();

