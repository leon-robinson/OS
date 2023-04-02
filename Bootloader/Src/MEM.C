#include "MEM.H"

void memcpy(void *dest, void *src, size_t n) {
	char *csrc = (char *)src;
	char *cdest = (char *)dest;

	for (size_t i = 0; i < n; i++)
		cdest[i] = csrc[i];
}

int memcmp(const void *s1, const void *s2, size_t n) {
	register const unsigned char *str1 = (const unsigned char *)s1;
	register const unsigned char *str2 = (const unsigned char *)s2;

	while (n-- > 0) {
		if (*str1++ != *str2++)
			return str1[-1] < str2[-1] ? -1 : 1;
	}

	return 0;
}
