#include <Drivers/VBE.H>

#include <Lib/GRAPHICS.H>
#include <Lib/MEM.H>

void DrawPoint(u16 x, u16 y, u32 color) {
	GetFirstBuffer()[GetWidth() * y + x] = color;
}

void FillRect(u16 x, u16 y, u16 w, u16 h, u32 color) {
	u16 width = GetWidth();
	u16 height = GetHeight();
	u32 *first_buffer = GetFirstBuffer();

	for (int i = 0; i < w; i++) {
		u16 xDest = i + x;

		if (xDest >= width)
			continue;

		for (int j = 0; j < h; j++) {
			u16 yDest = j + y;

			if (yDest >= height)
				continue;

			first_buffer[width * yDest + xDest] = color;
		}
	}
}

void FillScreen(u32 color) {
	MemsetU32(GetFirstBuffer(), color, GetSize() / 4);
}
