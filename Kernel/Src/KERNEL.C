#include <BOOT.H>

#include <Drivers/VBE.H>
#include <Drivers/IDT.H>
#include <Drivers/PIC.H>
#include <Drivers/PS2MOUSE.H>

#include <Lib/MEM.H>
#include <Lib/GRAPHICS.H>
#include <Lib/ASC16.H>
#include <Lib/TERMINAL.H>
#include <Lib/ALLOC.H>

#include <Mem/PMM.H>

#define SQUARE_SIZE 50

static u8 cursor[] = {
	1,0,0,0,0,0,0,0,0,0,0,0,
	1,1,0,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,1,1,1,1,1,
	1,2,2,2,1,2,2,1,0,0,0,0,
	1,2,2,1,0,1,2,2,1,0,0,0,
	1,2,1,0,0,1,2,2,1,0,0,0,
	1,1,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,0,1,1,0,0
};

static void DrawCursor(u16 x, u16 y) {
	for (int h = 0; h < 21; h++) {
		for (int w = 0; w < 12; w++) {
			u8 slot = cursor[h * 12 + w];

			if (slot == 1)
				DrawPoint(w + x, h + y, 0x00000000);
			else if (slot == 2)
				DrawPoint(w + x, h + y, 0xFFFFFFFF);
		}
	}
}

extern char __kernel_start;
extern char __kernel_end;

void Main(struct BootInfo *boot_info) {
	VBEInit(boot_info->vbe_info);

	IDTInit();
	PICInit();

	u64 kernel_size = (u64)&__kernel_end - (u64)&__kernel_start;

	PMMInit(boot_info->memory_info, kernel_size, boot_info->kernel_start);
	AllocInit();
	TerminalInit();
	VBEAllocBuffers();

	PS2MouseInit();

	for (; ; ) {
		FillScreen(DEFAULT_COLOR);
		TerminalUpdate();
		DrawCursor(GetMouseX(), GetMouseY());
		VBEUpdate();
	}
}
