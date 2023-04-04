
.PHONY: all
.PHONY: iso
.PHONY: clean
.PHONY: qemu

override QEMU_FLAGS := -enable-kvm -cpu host -m 4G -drive file=OS.iso,index=0,format=raw

all:
	@echo "Running Bootloader build tasks..."
	@$(MAKE) -C Bootloader --quiet

	@echo "Running Kernel build tasks..."
	@$(MAKE) -C Kernel --quiet

	@echo "Running ISO build task..."
	@$(MAKE) iso --quiet

	@echo "Starting QEMU..."
	@$(MAKE) qemu --quiet

iso:
	@rm -rf ISORoot
	@mkdir -p ISORoot
	@cp Bootloader/Obj/CD.BIN ISORoot/.
	@cp Kernel/Obj/KERNEL.BIN ISORoot/.
	@xorriso -as mkisofs -b CD.BIN \
		-no-emul-boot -boot-load-size 12 -boot-info-table \
		--protective-msdos-label \
		ISORoot -o OS.iso >/dev/null 2>&1
	@rm -rf ISORoot
	@dd if=Bootloader/Obj/HDD.BIN of=OS.iso bs=432 count=1 seek=0 conv=notrunc status=none
	@dd if=Bootloader/Obj/LOADBOOT.BIN of=OS.iso bs=512 count=1 seek=1 conv=notrunc status=none

clean:
	@echo "Removing OBJ directories..."
	@rm -rf $(shell find . -type d -name 'Obj')

	@echo "Removing ISO files..."
	@rm -rf $(shell find . -type f -name '*.iso')

qemu:
	@sudo qemu-system-x86_64 $(QEMU_FLAGS)