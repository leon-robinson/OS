#include <Mem/BITMAP.H>

void BitmapSet(u8 *bitmap, u64 index, boolean val) {
	if (val)
		bitmap[index / 8] |= (1 << (index % 8));
	else
		bitmap[index / 8] &= ~(1 << (index % 8));
}

boolean BitmapGet(u8 *bitmap, u64 index) {
	return bitmap[index / 8] & (1 << (index % 8));
}
