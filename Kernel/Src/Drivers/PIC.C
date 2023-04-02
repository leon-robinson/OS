#include <Drivers/PIC.H>

void PICInit() {
	OutU8(0x20, 0x11);
	OutU8(0xA0, 0x11);
	OutU8(0x21, 0x20);
	OutU8(0xA1, 40);
	OutU8(0x21, 0x04);
	OutU8(0xA1, 0x02);
	OutU8(0x21, 0x01);
	OutU8(0xA1, 0x01);

	OutU8(0x21, 0);
	OutU8(0xA1, 0);

	__asm__ volatile ("sti");
}

void PICEOI(u8 irq) {
	if (irq >= 40)
		OutU8(0xA0, 0x20);

	OutU8(0x20, 0x20);
}

void PICClearMask(u8 irq) {
	u16 port;
	u8 value;

	if (irq < 8)
		port = 0x21;
	else {
		port = 0xA1;
		irq -= 8;
	}

	value = InU8(port) & ~(1 << irq);
	OutU8(port, value);
}
