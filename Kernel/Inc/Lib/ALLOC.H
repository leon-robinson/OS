#pragma once

#include <TYPE.H>

void AllocInit();

struct Slab {
	u64 first_free;
	u64 size;
};
void SlabInit(struct Slab *slab, u64 size);
void *SlabAlloc(struct Slab *slab);
void SlabFree(struct Slab *slab, void *ptr);

struct SlabHeader {
	struct Slab *slab;
};

struct BigAllocMeta {
	u64 pages;
	u64 size;
};

extern struct Slab slabs[10];

struct Slab *GetSlab(u64 size); 
void *BigMalloc(u64 size);
void *BigRealloc(void *oldptr, u64 size);
void BigFree(void *ptr);
u64 BigAllocSize(void *ptr);

void *Malloc(u64 size);
void *Calloc(u64 num, u64 size);
void *Realloc(void *oldptr, u64 size);
void Free(void *ptr);
u64 AllocSize(void *ptr);
