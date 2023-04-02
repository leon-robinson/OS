#pragma once

#include <TYPE.H>

struct IDTEntry {
	u16 low;
	u16 selector;
	u8 res0;
	u8 attribute;
	u16 mid;
	u32 high;
	u32 res1;
} __attribute__((packed));

struct IDTR {
	u16 limit;
	u64 addr;
} __attribute__((packed));

struct InterruptFrame {
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	u64 r9;
	u64 r8;
	u64 rbp;
	u64 rdi;
	u64 rsi;
	u64 rdx;
	u64 rcx;
	u64 rbx;
	u64 rax;
	u64 trapno;
	u64 errorcode;
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} __attribute__((packed));

void Vector0();
void Vector1();
void Vector2();
void Vector3();
void Vector4();
void Vector5();
void Vector6();
void Vector7();
void Vector8();
void Vector10();
void Vector11();
void Vector12();
void Vector13();
void Vector14();
void Vector16();
void Vector17();
void Vector18();
void Vector19();
void Vector32();
void Vector33();
void Vector39();
void Vector44();
void LoadIDT(struct IDTR *ptr);
u8 ReadISR();

void IDTInit();
