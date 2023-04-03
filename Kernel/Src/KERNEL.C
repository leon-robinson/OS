#include <BOOT.H>

#include <Drivers/VBE.H>
#include <Drivers/IDT.H>
#include <Drivers/PIC.H>
#include <Drivers/PS2MOUSE.H>

#include <Lib/MEM.H>
#include <Lib/GRAPHICS.H>
#include <Lib/ASC16.H>
#include <Lib/TERMINAL.H>

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

void Main(struct BootInfo *boot_info) {
	VBEInit(boot_info->vbe_info);

	IDTInit();
	PICInit();

	PS2MouseInit();

	struct MemoryInfo *memory_info = boot_info->memory_info;
	u64 count = memory_info->entry_count;
	struct E820 *entries = (struct E820 *)&memory_info->entries;
	u64 total_mem = 0;

	for (u64 i = 0; i < count; i++) {
		struct E820 entry = entries[i];

		u64 addr = entry.address;
		u64 final_addr = addr + entry.length;
		u32 type = entry.type;

		char buf[16];
		ItoaU64(addr, buf, 16);
		Print(buf);
		ItoaU64(final_addr, buf, 16);
		Print("-");
		Print(buf);
		Print(": ");
		ItoaU64(type, buf, 16);
		Print(buf);
		Print("\n");

		total_mem += entry.length;
	}

	total_mem /= 1024 * 1024;
	char buf[16];
	ItoaU64(total_mem, buf, 10);
	Print("Total memory: ");
	Print(buf);
	Print("MiB\n\n");

	for (; ; ) {
		FillScreen(DEFAULT_COLOR);
		TerminalUpdate();
		DrawCursor(GetMouseX(), GetMouseY());
		VBEUpdate();
	}
}
