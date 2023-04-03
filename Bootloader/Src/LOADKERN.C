#include "LIB9660.H"
#include "MEM.H"
#include "TYPES.H"

l9660_fs fs;
l9660_dir dir;
l9660_file file;

static bool readsect(l9660_fs *fs, void *buf, uint32_t sector) {
	(void)fs;

	memcpy(buf, ((void *) 0xA00000 + (sector * 2048)), 2048);
	return true;
}

static void print(const char *str) {
	static uint8_t *vmem = (uint8_t *)0xB8000;
	static int column = 0;

	char *s = (char *)str;

	while (*s != 0) {
		if (*s == '\n') {
			column++;
		
			vmem = (uint8_t *)((uint8_t *)0xB8000 + (column * (80 * 2)));

			s++;

			continue;
		}

		*vmem = *s;
		*++vmem = 0xF;
	
		vmem++;
		s++;
	}
}

static void check(l9660_status stat) {
	if (stat) {
		print("ERROR LOADING KERNEL.");
		for (; ; );
	}
}

uint64_t LoadKernel() {
	check(l9660_openfs(&fs, readsect));
	check(l9660_fs_open_root(&dir, &fs));
	check(l9660_openat(&file, &dir, "KERNEL.BIN"));

	void *ptr = (void *)0xffffffff80000000;
	void *read = (void *)((uint8_t *)0xA00000 + (2048 * file.first_sector));
	memcpy(ptr, read, file.length);

	return file.length;
}