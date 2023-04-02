#include <Drivers/VBE.H>

#include <Lib/MEM.H>
#include <Lib/GRAPHICS.H>

static struct VBEInfo *vbe_info;

static u32 *video_memory = (u32 *)0xFFFF8000C0000000;
static u32 *first_buffer = NULL;
static u32 *second_buffer = NULL;
static u64 size = 0;

void VBEInit(struct VBEInfo *_vbe_info) {
	vbe_info = _vbe_info;

	size = vbe_info->width * vbe_info->height * vbe_info->bytes_pp;

	Memset(video_memory, 0, GetSize());

	first_buffer = (u32 *)0x100000;
	second_buffer = (u32 *)(first_buffer + GetSize());

	Memset(first_buffer, 0, GetSize());
	Memset(second_buffer, 0, GetSize());
}

void VBEUpdate() {
	for (u32 i = 0; i < vbe_info->width * vbe_info->height; i++) {
		if (first_buffer[i] != second_buffer[i]) {
			video_memory[i] = first_buffer[i];
		}
	}

	Memcpy(second_buffer, first_buffer, GetSize());
}

u16 GetWidth() {
	return vbe_info->width;
}

u16 GetHeight() {
	return vbe_info->height;
}

u16 GetPitch() {
	return vbe_info->pitch;
}

u32 GetBytesPP() {
	return vbe_info->bytes_pp;
}

u32 *GetFirstBuffer() {
	return first_buffer;
}

u64 GetSize() {
	return size;
}
