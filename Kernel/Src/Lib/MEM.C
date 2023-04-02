#include <Lib/MEM.H>
#include <Lib/MATH.H>

u32 Strlen(const char *str) {
	const char *p = str;
	for (; *p != '\0'; p++);
	return p - str;
}

static void Swap(char *x, char *y) {
	char t = *x; *x = *y; *y = t;
}

char *Reverse(char *str, u32 i, u32 j) {
	while (i < j)
		Swap(&str[i++], &str[j--]);
	return str;
}

char *ItoaU64(u64 val, char *str, int base) {
	char *rc;
	char *ptr;
	char *low;

	rc = ptr = str;
	low = ptr;

	do {
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJLMNOPQRSTUVWXYZ"[35 + val % base];
		val /= base;
	} while (val);

	*ptr-- = '\0';

	while (low < ptr) {
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

	return rc;
}