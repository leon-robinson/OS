#include <Drivers/IDT.H>
#include <Drivers/VBE.H>
#include <Drivers/PS2KEYBOARD.H>
#include <Drivers/PS2MOUSE.H>
#include <Drivers/PIC.H>

#include <Lib/IO.H>
#include <Lib/MEM.H>
#include <Lib/TERMINAL.H>
#include <Lib/GRAPHICS.H>
#include <Lib/X64.H>

static struct IDTR idtr;
static struct IDTEntry vectors[256];

static void AddIDTEntry(struct IDTEntry *entry, u64 addr, u8 attribute, char clear_mask) {
	entry->low = (u16)addr;
	entry->selector = 0x8;
	entry->attribute = attribute;
	entry->mid = (u16)(addr >> 16);
	entry->high = (u32)(addr >> 32);

	if (clear_mask != -1)
		PICClearMask((u8) clear_mask);
}

void IDTInit() {
	AddIDTEntry(&vectors[0], (u64)Vector0, 0x8E, -1);
	AddIDTEntry(&vectors[1], (u64)Vector1, 0x8E, -1);
	AddIDTEntry(&vectors[2], (u64)Vector2, 0x8E, -1);
	AddIDTEntry(&vectors[3], (u64)Vector3, 0x8E, -1);
	AddIDTEntry(&vectors[4], (u64)Vector4, 0x8E, -1);
	AddIDTEntry(&vectors[5], (u64)Vector5, 0x8E, -1);
	AddIDTEntry(&vectors[6], (u64)Vector6, 0x8E, -1);
	AddIDTEntry(&vectors[7], (u64)Vector7, 0x8E, -1);
	AddIDTEntry(&vectors[8], (u64)Vector8, 0x8E, -1);
	AddIDTEntry(&vectors[10], (u64)Vector10, 0x8E, -1);
	AddIDTEntry(&vectors[11], (u64)Vector11, 0x8E, -1);
	AddIDTEntry(&vectors[12], (u64)Vector12, 0x8E, -1);
	AddIDTEntry(&vectors[13], (u64)Vector13, 0x8E, -1);
	AddIDTEntry(&vectors[14], (u64)Vector14, 0x8E, -1);
	AddIDTEntry(&vectors[16], (u64)Vector16, 0x8E, -1);
	AddIDTEntry(&vectors[17], (u64)Vector17, 0x8E, -1);
	AddIDTEntry(&vectors[18], (u64)Vector18, 0x8E, -1);
	AddIDTEntry(&vectors[19], (u64)Vector19, 0x8E, -1);
	AddIDTEntry(&vectors[32], (u64)Vector32, 0x8E, 32);
	AddIDTEntry(&vectors[33], (u64)Vector33, 0x8E, 33);
	AddIDTEntry(&vectors[39], (u64)Vector39, 0x8E, -1);
	AddIDTEntry(&vectors[44], (u64)Vector44, 0x8E, 44);

	idtr.limit = sizeof(vectors) - 1;
	idtr.addr = (u64)vectors;
	LoadIDT(&idtr);

	__asm__ volatile ("sti");
}

static void PrintReg(const char *name, u64 reg) {
	static char buf[16];
	char *t = ItoaU64(reg, buf, 16);
	Print("\n  ");
	Print(name);
	Print("=");
	Print(t);
}

void InterruptHandler(struct InterruptFrame *f) {
	u8 isr_value;

	switch (f->trapno) {
		case 0x20:
			PICEOI(f->trapno);
			break;

		case 0x21:
			u8 key = InU8(0x60);
			KeyboardHandler(key);
			PICEOI(f->trapno);
			break;

		case 0x2C:
			MouseOnInterrupt();
			PICEOI(f->trapno);
			break;

		case 39:
			isr_value = ReadISR();
			if ((isr_value & (1 << 7)) != 0)
				PICEOI(f->trapno);
			break;

		default:
			FillScreen(0);
			Clear();

			char buf[16];
			char *t = ItoaU64(f->rip, buf, 16);
			Print("CPU EXCEPTION, FAILED @ 0x");
			Print(t);

			t = ItoaU64(f->trapno, buf, 16);
			Print("\nEXCEPTION=0x");
			Print(t);

			if (f->trapno == 0xE) {
				u64 cr2 = GetCR2();
				t = ItoaU64(cr2, buf, 16);
				Print("\nPAGE FAULT ADDRESS=0x");
				Print(t);
			}

			Print("\n\nINTERRUPT FRAME:");
			PrintReg("SS", f->ss);
			PrintReg("RSP", f->rsp);
			PrintReg("RFLAGS", f->rflags);
			PrintReg("CS", f->cs);
			PrintReg("ERRORCODE", f->errorcode);
			PrintReg("RAX", f->rax);
			PrintReg("RBX", f->rbx);
			PrintReg("RCX", f->rcx);
			PrintReg("RDX", f->rdx);
			PrintReg("RSI", f->rsi);
			PrintReg("RDI", f->rdi);
			PrintReg("RBP", f->rbp);
			PrintReg("R8", f->r8);
			PrintReg("R9", f->r9);
			PrintReg("R10", f->r10);
			PrintReg("R11", f->r11);
			PrintReg("R12", f->r12);
			PrintReg("R13", f->r13);
			PrintReg("R14", f->r14);
			PrintReg("R15", f->r15);

			TerminalUpdate();
			VBEUpdate();
			for (; ; );
	}
}
