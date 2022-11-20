
override OVMF_DIR := boot/uefi/ovmf

.PHONY: all
.PHONY: iso
.PHONY: qemu
.PHONY: clean

all:
	@$(MAKE) -C boot/bios --quiet
	@$(MAKE) -C boot/uefi --quiet
	@$(MAKE) iso --quiet
	@$(MAKE) qemu-bios --quiet

iso:
	@echo "Generating ISO..."
	rm -rf iso_root
	mkdir -p iso_root
	cp bin/boot/bios/boot-cd.bin iso_root/boot-cd.bin
	cp bin/boot/uefi/uefi-cd.img iso_root/uefi-cd.img
	xorriso -as mkisofs -b boot-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot uefi-cd.img \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o OS.iso
	rm -rf iso_root

qemu-bios:
	sudo qemu-system-x86_64 -cdrom OS.iso -enable-kvm -cpu host -m 1G -boot d

qemu-uefi:
	sudo qemu-system-x86_64 -cdrom OS.iso -enable-kvm -cpu host -m 1G -boot d \
		-drive if=pflash,format=raw,unit=0,file="$(OVMF_DIR)/OVMF_CODE-pure-efi.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="$(OVMF_DIR)/OVMF_VARS-pure-efi.fd"

clean:
	@rm -rf bin
	@$(MAKE) -C boot/uefi/gnu-efi clean --quiet
	@echo "Cleaned."
