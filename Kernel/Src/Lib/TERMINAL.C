#include <Lib/TERMINAL.H>
#include <Lib/MEM.H>
#include <Lib/ASC16.H>
#include <Lib/GRAPHICS.H>
#include <Lib/IO.H>

#include <Drivers/VBE.H>

#define TOTAL_LINES 64
#define LINE_MAX_CHARS 80

static volatile int line_index = 0;

static const char lines[TOTAL_LINES][LINE_MAX_CHARS];

static boolean scrolling = false;

static int GetAvailableLines() {
	return (GetHeight() / 16) - 3;
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

			while (line_index > GetAvailableLines() - 1) {
				line_index--;
				Scroll();
			}
		}
	}
}

void Enter() {
	char *cmd = (char *)lines[line_index];
	char *cmd2 = cmd + 2;

	u32 cmd_len = Strlen(cmd);
	static char cmd_clone[LINE_MAX_CHARS];
	Memset(cmd_clone, 0, sizeof(cmd_clone));

	Memcpy(cmd_clone, cmd, cmd_len);

	Memmove((void *)cmd2, (void *)cmd, cmd_len);
	cmd[0] = '>';
	cmd[1] = ' ';

	Print("\n");

	if (Memcmp(cmd_clone, "clear", 5) == 0)
		Clear();
	else if (Memcmp(cmd_clone, "reboot", 6) == 0) {
		Print("Rebooting...");

		__asm__ volatile ("cli");

		u8 tmp = 0x02;
		while (tmp & 0x02)
			tmp = InU8(0x64);

		OutU8(0x64, 0xFE);

		__asm__ volatile ("hlt");
	} else {
		if (cmd_len != 0)
			Print("Unknown command!");
		Print("\n");
	}
}

void TerminalUpdate() {
	u16 y = 16;

	int available_lines = GetAvailableLines();

	for (int i = 0; i < available_lines; i++) {
		const char *line = (const char *)lines[i];
		if (line == NULL) break;

		u32 len = Strlen(line);

		u16 x = 10;

		if (i == line_index) {
			ASC16DrawChar('>', x, y, 0xFFFFFFFF);
			ASC16DrawChar(' ', x + 8, y, 0xFFFFFFFF);
			x += 16;
		}

		for (u32 j = 0; j < len; j++) {
			char c = line[j];
			ASC16DrawChar(c, x, y, 0xFFFFFFFF);
			x += 8;
		}

		y += 16;
	}

	int cursor_y = ((line_index * 16) + y) - (GetHeight() - (16 * 3));
	int cursor_x = (Strlen(lines[line_index]) * 8) + 10 + 16;
	FillRect(cursor_x, cursor_y, 2, 16, 0xFFFFFFFF);
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
		} else if (c == '\b') {
			c = '\0';
			n--;
		}

		u8 *ptr = (u8 *)(lines[line_index]) + n;
		*ptr = c;

		n++;
	}
}

void Clear() {
	Memset((void *)lines, 0, sizeof(lines));

	line_index = 0;
	scrolling = false;
}
