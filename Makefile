.PHONY: all clean toolchain kernel limine iso

# dirs

KERNEL=kernel
LIMINE=libs/limine
TOOLS=tools
TOOLCHAIN=toolchain

# misc

KERNEL_BUILD=kernel.sys
ISO_BUILD=os.iso

all: clean kernel limine iso

kernel:
	@make -C $(KERNEL) KERNEL_BUILD=$(abspath $(KERNEL_BUILD)) all

limine:
	@make -C $(LIMINE) all

toolchain:
	@cd $(TOOLCHAIN) && ./get-toolchain.sh

iso:
	@$(TOOLS)/build-iso.sh $(LIMINE) $(ISO_BUILD)

clean:
	@make -C $(KERNEL) KERNEL_BUILD=$(abspath $(KERNEL_BUILD)) clean
	
	@rm -rf $(ISO_BUILD)