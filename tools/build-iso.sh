#!/bin/bash

mkdir -p iso/boot/limine/ iso/EFI/BOOT/

cp $1/limine-bios-cd.bin $1/limine-bios.sys $1/limine-uefi-cd.bin iso/boot/limine/
cp $1/BOOTX64.EFI iso/EFI/BOOT/

cp limine.conf iso/boot/limine.conf
cp kernel.sys iso/boot/kernel.sys
cp README.md iso/boot/test.txt

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
    -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image --protective-msdos-label \
    iso/ -o $2

rm -rf iso

$1/limine bios-install $2