#include <Lib/TERMINAL.H>
#include <Lib/MEM.H>
#include <Lib/ASC16.H>
#include <Lib/GRAPHICS.H>

#include <Drivers/VBE.H>

#define TOTAL_LINES 64
#define LINE_MAX_CHARS 80

static volatile int line_index = 0;

static const char lines[TOTAL_LINES][LINE_MAX_CHARS];

static boolean scrolling = false;

static int GetAvailableLines() {
	return (GetHeight() / 16) - 3;
}

void TerminalUpdate() {
	u16 y = 16;

	int available_lines = GetAvailableLines();

	for (int i = 0; i < available_lines; i++) {
		const char *line = (const char *)lines[i];
		if (line == NULL) break;

		u32 len = Strlen(line);

		u16 x = 10;

		for (u32 j = 0; j < len; j++) {
			char c = line[j];
			ASC16DrawChar(c, x, y, 0xFFFFFFFF);
			x += 8;
		}

		y += 16;
	}

	int cursor_y = ((line_index * 16) + y) - (GetHeight() - (16 * 3));
	int cursor_x = (Strlen(lines[line_index]) * 8) + 10;
	FillRect(cursor_x, cursor_y, 2, 16, 0xFFFFFFFF);
}

static void Scroll() {
	for (int i = 1; i < 64; i++)
		Memmove((void *) lines[i - 1], (void *) lines[i], 80);
}

static void NewLine() {
	line_index++;

	if (line_index > GetAvailableLines()) {
		if (!scrolling) {
			scrolling = true;
			line_index-=2;
			Scroll();
		}
	}
}

void Print(const char *str) {
	int n = 0;

	u32 str_len = Strlen((const char *) lines[line_index]);
	if (str_len)
		n = str_len;

	for (u32 i = 0; i < Strlen(str); i++) {
		char c = str[i];

		if (c == '\n') {
			n = 0;
			if (scrolling)
				Scroll();
			else
				NewLine();
			continue;
		}

		u8 *ptr = (u8 *)(lines[line_index]) + n;
		*ptr = c;

		n++;
	}
}

void Clear() {
	for (int i = 0; i < TOTAL_LINES; i++)
		Memset((void *) lines[i], 0, LINE_MAX_CHARS);

	line_index = 0;
	scrolling = false;
}
