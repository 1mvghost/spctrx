#!/bin/bash
unset GTK_PATH 
qemu-system-x86_64 -cdrom ./os.iso -smp 16 -cpu qemu64 -m 4G -boot order=d -bios /usr/share/ovmf/OVMF.fd -net none -serial stdio -accel kvm -M q35