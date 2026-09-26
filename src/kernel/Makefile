.PHONY: all tmp

#Dirs

BUILD?=./build
KERNEL_BUILD?=./

INC=inc

SRC_DIRS=$(shell find ./* -type d)
TMP_DIRS=$(patsubst ./%, $(BUILD)/%, $(SRC_DIRS))

#Tools

C=clang
C_FLAGS=\
	-I$(INC)/ \
	-static \
	-pipe \
	-g \
	-std=gnu11 \
	-ffreestanding \
	-m64 \
	-nostdlib \
	-Wall \
	-Wextra \
	-Werror \
	-mcmodel=kernel \
	-mno-red-zone \
	-march=x86-64 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-fno-common \
	-fno-pic \
	-fno-stack-protector \
	-fno-stack-check \
	-ffunction-sections \
	-fdata-sections

LD=ld.lld
LD_FLAGS=\
	-T link.ld \
	-nostdlib \
	-z max-page-size=4096

ASM=nasm
ASM_FLAGS=\
	-f elf64

#Misc

ASM_SOURCE=$(shell find . -name "*.asm")
ASM_OBJ=$(patsubst ./%.asm, $(BUILD)/%.asm.o, $(ASM_SOURCE))

C_SOURCE=$(shell find . -name "*.c")
C_OBJ=$(patsubst ./%.c, $(BUILD)/%.c.o, $(C_SOURCE))

tmp: 
	@mkdir -p $(TMP_DIRS) # create all tmp dirs

all: tmp $(KERNEL_BUILD)

$(BUILD)/%.asm.o: %.asm
	@echo "* asm: $<"
	@$(ASM) $(ASM_FLAGS) $< -o $@

$(BUILD)/%.c.o: %.c
	@echo "* c:   $<"
	@$(C) $(C_FLAGS) $< -o $@ -c

$(KERNEL_BUILD): $(ASM_OBJ) $(C_OBJ)
	@$(LD) -o $@ $^ $(LD_FLAGS)