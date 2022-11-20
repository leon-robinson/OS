
.PHONY: all
.PHONY: iso
.PHONY: qemu
.PHONY: clean

all:
	@$(MAKE) -C boot/bios --quiet
	@$(MAKE) iso --quiet
	@$(MAKE) qemu --quiet

iso:
	@echo "Generating ISO..."
	@rm -rf iso_root
	@mkdir -p iso_root
	@cp bin/boot/bios/boot-cd.bin iso_root/boot-cd.bin
	@xorriso -as mkisofs -b boot-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--protective-msdos-label \
		iso_root -o OS.iso
	@rm -rf iso_root

qemu:
	@qemu-system-x86_64 -cdrom OS.iso -enable-kvm -cpu host -m 1G -boot d

clean:
	@rm -rf bin
