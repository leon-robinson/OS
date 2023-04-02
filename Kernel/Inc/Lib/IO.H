#pragma once

#include <TYPE.H>

static inline void OutU8(u16 port, u8 data) {
	__asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline u8 InU8(u16 port) {
	u8 data;
	__asm__ volatile ("inb %1, %0"
				: "=a"(data)
				: "Nd"(port));
	return data;
}

static inline void OutU16(u16 port, u16 data) {
	__asm__ volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

static inline u16 InU16(u16 port) {
	u16 data;
	__asm__ volatile ("inw %1, %0"
				: "=a"(data)
				: "Nd"(port));
	return data;
}

static inline void OutU32(u16 port, u32 data) {
	__asm__ volatile ("outl %0, %1" : : "a"(data), "Nd"(port));
}

static inline u32 InU32(u16 port) {
	u32 data;
	__asm__ volatile ("inl %1, %0"
				: "=a"(data)
				: "Nd"(port));
	return data;
}
