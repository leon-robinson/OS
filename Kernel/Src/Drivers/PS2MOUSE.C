#include <Drivers/PS2MOUSE.H>
#include <Drivers/VBE.H>
#include <Drivers/IDT.H>

#include <Lib/MATH.H>
#include <Lib/IO.H>
#include <Lib/GRAPHICS.H>

static u16 x;
static u16 y;

static u8 phase = 0;
static u8 data[3] = {0,0,0};
static short aX;
static short aY;

static ALWAYS_INLINE inline void Wait() {
	__asm__ volatile ("hlt");
}

static u8 ReadRegister() {
	Wait();
	return InU8(PS2_DATA);
}

static void WriteRegister(u8 val) {
	Wait();
	OutU8(PS2_CMD, 0xD4);
	Wait();
	OutU8(PS2_DATA, val);

	ReadRegister();
}

void PS2MouseInit() {
	u8 status;

	Wait();
	OutU8(PS2_CMD, 0xA8);

	Wait();
	OutU8(PS2_CMD, 0x20);
	Wait();
	status = InU8(PS2_DATA) | 3;
	Wait();
	OutU8(PS2_CMD, 0x60);
	Wait();
	OutU8(PS2_DATA, status);

	WriteRegister(PS2_SET_DEFAULTS);
	WriteRegister(PS2_ENABLE_DATA_REPORTING);

	WriteRegister(0xF2);

	WriteRegister(0xF3);
	WriteRegister(200);

	WriteRegister(0xF2);
}

void MouseOnInterrupt() {
	u8 d = InU8(PS2_DATA);

	if (phase == 0) {
		if (d == 0xFA)
			phase = 1;
	} else if (phase == 1) {
		if ((d & 8) == 8) {
			data[0] = d;
			phase = 2;
		}
	} else if (phase == 2) {
		data[1] = d;
		phase = 3;
	} else if (phase == 3) {
		data[2] = d;
		phase = 1;

		data[0] &= 0x07;

		aX = (char)data[1];
		aY = (char)data[2];
		aY = -aY;

		x = ClampU16(x + aX, 0, GetWidth());
		y = ClampU16(y + aY, 0, GetHeight());
	}
}

u16 GetMouseX() {
	return x;
}

u16 GetMouseY() {
	return y;
}
