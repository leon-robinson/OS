
override BIN_DIR := ../../bin/boot/bios

.PHONY: all
all:
	@mkdir -p $(BIN_DIR)
	@nasm -f bin boot-cd.asm -o $(BIN_DIR)/boot-cd.bin
