#include <Mem/PMM.H>
#include <Mem/BITMAP.H>

#include <Lib/MATH.H>
#include <Lib/MEM.H>
#include <Lib/TERMINAL.H>

#include <Drivers/VBE.H>

static u64 mem_usable_top = 0;
static u64 last_index = 0;
static u8 *bitmap = NULL;

static u64 usable_mem = 0;
static u64 total_mem = 0;
static u64 used_mem = 0;

u64 mem_top = 0;

void PMMInit(struct MemoryInfo *mem_info, u64 kernel_size, u64 kernel_start) {
	u64 count = mem_info->entry_count;
	struct E820 *entries = (struct E820 *)&mem_info->entries;
	u64 total_mem = 0;

	for (u64 i = 0; i < count; i++) {
		struct E820 entry = entries[i];

		u64 top = entry.address + entry.length;

		if (top > mem_top)
			mem_top = top;

		if (entry.type == 1) {
			usable_mem += entry.length;
			if (top > mem_usable_top)
				mem_usable_top = top;
		}

		total_mem += entry.length;
	}

	u64 bitmap_size = ALIGN_UP((mem_usable_top / PAGE_SIZE) / 8, PAGE_SIZE);

	for (u64 i = 0; i < count; i++) {
		struct E820 entry = entries[i];

		if (entry.type != 1)
			continue;

		if (entry.length >= bitmap_size && entry.address != 0) {
			bitmap = (u8 *)entry.address;
			Memset((void *)bitmap, 0xFF, bitmap_size);
			break;
		}
	}

	if (bitmap == NULL) {
		Print("Failed to find location for bitmap.");
		TerminalUpdate();
		VBEUpdate();
		for (; ; );
	}

	for (u64 i = 0; i < count; i++) {
		struct E820 entry = entries[i];

		if (entry.type != 1)
			continue;

		for (u64 t = 0; t < entry.length; t += PAGE_SIZE)
			BitmapSet(bitmap, (entry.address + t) / PAGE_SIZE, false);
	}

	// Lock 0-40K.
	for (u64 i = 0; i < (0x40000 / PAGE_SIZE); i++)
		BitmapSet(bitmap, i, true);

	// Lock bitmap.
	u64 bitmap_low = ALIGN_DOWN((u64)bitmap, PAGE_SIZE) / PAGE_SIZE;
	u64 bitmap_high = ALIGN_UP((u64)bitmap + bitmap_size, PAGE_SIZE) / PAGE_SIZE;
	for (u64 i = bitmap_low; i < bitmap_high; i++)
		BitmapSet(bitmap, i, true);

	// Lock kernel.
	u64 kernel_low = ALIGN_DOWN(kernel_start, PAGE_SIZE) / PAGE_SIZE;
	u64 kernel_high = ALIGN_UP(kernel_start + kernel_size, PAGE_SIZE) / PAGE_SIZE;
	for (u64 i = kernel_low; i < kernel_high; i++)
		BitmapSet(bitmap, i, true);

	mem_top = ALIGN_UP(mem_top, 0x40000000);
}

static void *InnerAlloc(u64 count, u64 limit) {
	u64 p = 0;

	while (last_index < limit) {
		if (!BitmapGet(bitmap, last_index++)) {
			if (++p == count) {
				u64 page = last_index - count;
				for (size_t i = page; i < last_index; i++)
					BitmapSet(bitmap, i, true);
				return (void *)(page * PAGE_SIZE);
			}
		} else {
			p = 0;
		}
	}

	return NULL;
}

void *AllocPage(u32 count) {
	if (count == 0)
		return NULL;

	u64 i = last_index;
	void *ret = InnerAlloc(count, mem_usable_top / PAGE_SIZE);
	if (ret == NULL) {
		last_index = 0;
		ret = InnerAlloc(count, i);
		if (ret == NULL) {
			//Clear();
			Print("Out of memory.");
			TerminalUpdate();
			VBEUpdate();
			for (; ; );
		}
	}

	Memset(ret, 0, count * PAGE_SIZE);

	used_mem += count * PAGE_SIZE;
	return ret;
}

void FreePage(void *ptr, u32 count) {
	if (ptr == NULL)
		return;

	u64 page = (u64)(ptr) / PAGE_SIZE;
	for (u64 i = page; i < page + count; i++)
		BitmapSet(bitmap, i, false);

	if (page < last_index)
		last_index = page;

	used_mem -= count * PAGE_SIZE;
}

u64 GetTotalMem() {
	return total_mem;
}

u64 GetUsableMem() {
	return usable_mem;
}

u64 GetUsedMem() {
	return used_mem;
}

u64 GetFreeMem() {
	return usable_mem - used_mem;
}
