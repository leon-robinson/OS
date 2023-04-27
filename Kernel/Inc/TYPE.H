#pragma once

#define ALWAYS_INLINE __attribute__((always_inline))
#define PACKED __attribute__((packed))

typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;

#define true 1
#define false 0

typedef u8 boolean;

typedef unsigned long size_t;

#define NULL ((void *) 0)