#!/bin/bash
unset GTK_PATH
qemu-system-x86_64 -cdrom ./os.iso -smp 16 -m 4G -M q35 -vga vmware -serial stdio -boot order=d -accel kvm