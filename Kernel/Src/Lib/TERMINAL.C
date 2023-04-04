#include <Lib/TERMINAL.H>
#include <Lib/MEM.H>
#include <Lib/ASC16.H>
#include <Lib/GRAPHICS.H>
#include <Lib/IO.H>
#include <Lib/MATH.H>
#include <Lib/ALLOC.H>

#include <Mem/PMM.H>

#include <Drivers/VBE.H>

static volatile int line_index = 0;
static const char **lines;
static boolean scrolling = false;
static u64 total_lines = 0;
static u64 line_size = 0;

static int GetAvailableLines() {
	u16 height = GetHeight();
	return ((height - (height % 16)) / 16) - 3;
}

void TerminalInit() {
	total_lines = ALIGN_UP(GetHeight(), 16) / 16;
	line_size = ALIGN_UP(GetWidth(), 8) / 8;
	lines = Calloc(total_lines, line_size);
}

const char *GetLine(int i) {
	u8 *ptr = (u8 *)lines;
	ptr += i * line_size;
	return (const char *) ptr;
}

static void Scroll() {
	for (u64 i = 1; i < total_lines; i++)
		Memmove((void *) GetLine(i - 1), (void *) GetLine(i), line_size);
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
	char *cmd = (char *)GetLine(line_index);
	char *cmd2 = cmd + 2;

	u32 cmd_len = Strlen(cmd);
	char *cmd_clone = Malloc(line_size);
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
	} else if (Memcmp(cmd_clone, "allocpage", 9) == 0) {
		void *page = AllocPage(1);
		char buf[16];
		ItoaU64((u64) page, buf, 16);
		Print("Allocated page at: ");
		Print(buf);
		Print("\n");
	} else {
		if (cmd_len != 0)
			Print("Unknown command!\n");
	}

	Free(cmd_clone);
}

void TerminalUpdate() {
	u16 y = 16;

	int available_lines = GetAvailableLines();

	for (int i = 0; i < available_lines; i++) {
		const char *line = (const char *)GetLine(i);
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

	int cursor_y = (line_index * 16) + 16;
	int cursor_x = (Strlen(GetLine(line_index)) * 8) + 10 + 16;
	FillRect(cursor_x, cursor_y, 2, 16, 0xFFFFFFFF);
}

void Print(const char *str) {
	int n = 0;

	u32 str_len = Strlen((const char *) GetLine(line_index));
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

		u8 *ptr = (u8 *)(GetLine(line_index)) + n;
		*ptr = c;

		n++;
	}
}

void Clear() {
	Memset((void *)lines, 0, total_lines * line_size);

	line_index = 0;
	scrolling = false;
}
