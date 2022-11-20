
override BIN_DIR := ../../bin/boot/uefi

.PHONY: all
.PHONY: gnu-efi

all:
	$(MAKE) gnu-efi --quiet
	mkdir -p $(BIN_DIR)
	gcc -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c src/main.c -o $(BIN_DIR)/main.c.o
	ld -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $(BIN_DIR)/main.c.o -o $(BIN_DIR)/main.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(BIN_DIR)/main.so $(BIN_DIR)/BOOTX64.EFI
	dd if=/dev/zero of=$(BIN_DIR)/uefi-cd.img bs=1k count=1440 status=none
	mformat -i $(BIN_DIR)/uefi-cd.img -f 1440 ::
	mmd -i $(BIN_DIR)/uefi-cd.img ::/EFI
	mmd -i $(BIN_DIR)/uefi-cd.img ::/EFI/BOOT
	mcopy -i $(BIN_DIR)/uefi-cd.img $(BIN_DIR)/BOOTX64.EFI ::/EFI/BOOT

gnu-efi:
	@echo "[make] gnu-efi"
	$(MAKE) -C gnu-efi --quiet