.PHONY: all clean kernel limine iso

# dirs

KERNEL=kernel
ISO=iso
LIMINE=limine
TOOLS=tools

# misc

KERNEL_BUILD=kernel.sys
ISO_BUILD=os.iso

all: clean kernel limine iso

kernel:
	@make -C $(KERNEL) KERNEL_BUILD=$(abspath $(KERNEL_BUILD)) all

limine:
	@make -C $(LIMINE) all

iso:
	@mkdir -p $(ISO)/boot/limine/ $(ISO)/EFI/BOOT

	@cp $(LIMINE)/limine-bios-cd.bin $(LIMINE)/limine-bios.sys $(LIMINE)/limine-uefi-cd.bin $(ISO)/boot/limine/
	@cp $(LIMINE)/BOOTX64.EFI $(LIMINE)/BOOTIA32.EFI $(ISO)/EFI/BOOT/
	@cp limine.conf $(ISO)/boot/limine/limine.conf

	@cp $(KERNEL_BUILD) $(ISO)/boot/kernel.sys

	@xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(ISO)/ -o $(ISO_BUILD)
	
	@$(LIMINE)/limine bios-install $(ISO_BUILD)

	@echo "*** iso ready: $(ISO_BUILD)"

clean:
	rm -rf $(ISO_BUILD) $(ISO)