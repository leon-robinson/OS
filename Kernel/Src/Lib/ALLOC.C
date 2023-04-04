#include <Lib/ALLOC.H>
#include <Lib/MATH.H>
#include <Lib/MEM.H>

#include <Mem/PMM.H>

struct Slab slabs[10];

void AllocInit() {
	SlabInit(&slabs[0], 8);
	SlabInit(&slabs[1], 16);
	SlabInit(&slabs[2], 24);
	SlabInit(&slabs[3], 32);
	SlabInit(&slabs[4], 48);
	SlabInit(&slabs[5], 64);
	SlabInit(&slabs[6], 128);
	SlabInit(&slabs[7], 256);
	SlabInit(&slabs[8], 512);
	SlabInit(&slabs[9], 1024);
}

void SlabInit(struct Slab *slab, u64 size) {
	slab->size = size;
	slab->first_free = (u64) AllocPage(1);

	u64 available = 0x1000 - ALIGN_UP(sizeof(struct SlabHeader), slab->size);
	struct SlabHeader *slab_ptr = (struct SlabHeader *)(slab->first_free);
	slab_ptr->slab = slab;
	slab->first_free += ALIGN_UP(sizeof(struct SlabHeader), slab->size);

	u64 *arr = (u64 *)(slab->first_free);
	u64 max = available / slab->size - 1;
	u64 fact = slab->size / 8;

	for (u64 i = 0; i < max; i++)
		arr[i * fact] = (u64)(&arr[(i + 1) * fact]);

	arr[max * fact] = 0;
}

void *SlabAlloc(struct Slab *slab) {
	if (slab->first_free == 0)
		SlabInit(slab, slab->size);

	u64 *old_free = (u64 *)(slab->first_free);
	slab->first_free = old_free[0];
	Memset(old_free, 0, slab->size);
	return old_free;
}

void SlabFree(struct Slab *slab, void *ptr) {
	if (ptr == NULL)
		return;

	u64 *new_head = (u64 *)(ptr);
	new_head[0] = slab->first_free;
	slab->first_free = (u64)(new_head);
}

struct Slab slabs[10];

struct Slab *GetSlab(u64 size) {
	for (int i = 0; i < 10; i++) {
		struct Slab slab = slabs[i];
		if (slab.size >= size)
			return &slabs[i];
	}

	return NULL;
}

void *BigMalloc(u64 size) {
	u64 pages = DIV_ROUNDUP(size, 0x1000);
	void *ptr = AllocPage(pages + 1);

	struct BigAllocMeta *meta_data = (struct BigAllocMeta *)(ptr);
	meta_data->pages = pages;
	meta_data->size = size;
	return (void *)((u64)(ptr) + 0x1000);
}

void *BigRealloc(void *oldptr, u64 size) {
	if (oldptr == NULL)
		return Malloc(size);

	struct BigAllocMeta *meta_data = (struct BigAllocMeta *)((u64)(oldptr) - 0x1000);
	u64 old_size = meta_data->size;

	if (DIV_ROUNDUP(old_size, 0x1000) == DIV_ROUNDUP(size, 0x1000)) {
		meta_data->size = size;
		return oldptr;
	}

	if (size == 0) {
		Free(oldptr);
		return NULL;
	}

	if (size < old_size)
		old_size = size;

	void *new_ptr = Malloc(size);
	if (new_ptr == NULL)
		return oldptr;

	Memcpy(new_ptr, oldptr, old_size);
	Free(oldptr);
	return new_ptr;
}

void BigFree(void *ptr) {
	struct BigAllocMeta *meta_data = (struct BigAllocMeta *)((u64)(ptr) - 0x1000);
	FreePage(meta_data, meta_data->pages + 1);
}

u64 BigAllocSize(void *ptr) {
	return ((struct BigAllocMeta *)((u64)(ptr) - 0x1000))->size;
}

void *Malloc(u64 size) {
	struct Slab *slab = GetSlab(size);
	if (slab == NULL)
		return BigMalloc(size);
	return SlabAlloc(slab);
}

void *Calloc(u64 num, u64 size) {
	void *ptr = Malloc(num * size);
	if (ptr == NULL)
		return NULL;

	Memset(ptr, 0, num * size);
	return ptr;
}

void *Realloc(void *oldptr, u64 size) {
	if (oldptr == NULL)
		return Malloc(size);

	if (((u64)(oldptr) & 0xFFF) == 0)
		return BigRealloc(oldptr, size);

	struct Slab *slab = ((struct SlabHeader *)((u64)(oldptr) & ~0xFFF))->slab;
	u64 old_size = slab->size;

	if (size == 0) {
		Free(oldptr);
		return NULL;
	}

	if (size < old_size)
		old_size = size;

	void *new_ptr = Malloc(size);
	if (new_ptr == NULL)
		return oldptr;

	Memcpy(new_ptr, oldptr, old_size);
	Free(oldptr);
	return new_ptr;
}

void Free(void *ptr) {
	if (ptr == NULL)
		return;

	if (((u64)(ptr) & 0xFFF) == 0)
		return BigFree(ptr);

	struct SlabHeader *slab_hdr = (struct SlabHeader *)((u64)(ptr) & ~0xFFF);
	struct Slab *slab = slab_hdr->slab;
	SlabFree(slab, ptr);
}

u64 AllocSize(void *ptr) {
	if (ptr == NULL)
		return 0;

	if (((u64)(ptr) & 0xFFF) == 0)
		return BigAllocSize(ptr);

	struct SlabHeader *slab_hdr = (struct SlabHeader *)((u64)(ptr) & ~0xFFF);
	struct Slab *slab = slab_hdr->slab;
	return slab->size;
}
