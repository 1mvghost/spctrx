.PHONY: all createTmpDirs format clean

KERNEL_BUILD?=./

# dirs

LIBS=libs
SRC=src
BUILD=build

INCLUDE=$(SRC)/include

SRC_DIRS=$(shell find $(LIBS) $(SRC) -type d)
TMP_DIRS=$(patsubst %, $(BUILD)/%, $(SRC_DIRS))

# tools

TOOLCHAIN=x86_64-elf

CC=$(TOOLCHAIN)-gcc
LD=$(TOOLCHAIN)-ld
ASM=nasm
FORMAT=clang-format

# flags

CC_FLAGS=\
	-I$(INCLUDE)/ \
	-I$(LIBS)/flanterm/src/ \
	-static \
	-pipe \
	-g \
	-O2 \
	-std=gnu11 \
	-ffreestanding \
	-m64 \
	-nostdlib \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Werror \
	-mcmodel=kernel \
	-mno-red-zone \
	-march=x86-64 \
	-mabi=sysv \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-fno-common \
	-fno-pic \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-ffunction-sections \
	-fdata-sections

LD_FLAGS=\
	-m elf_x86_64 \
	-T link.ld \
	-nostdlib \
	-static \
	-z noexecstack \
	-z max-page-size=4096 \
    --gc-sections

ASM_FLAGS=\
	-f elf64 \
	-g

# misc

ASM_SOURCE=$(shell find $(LIBS) $(SRC) -name "*.asm")
ASM_OBJ=$(patsubst %.asm, $(BUILD)/%.asm.o, $(ASM_SOURCE))

C_SOURCE=$(shell find $(LIBS) $(SRC) -name "*.c")
C_OBJ=$(patsubst %.c, $(BUILD)/%.c.o, $(C_SOURCE))

all: clean createTmpDirs format $(KERNEL_BUILD)

createTmpDirs: 
	@mkdir -p $(TMP_DIRS)

$(BUILD)/%.asm.o: %.asm
	@echo "* ASM: $<"
	@$(ASM) $(ASM_FLAGS) $< -o $@

$(BUILD)/%.c.o: %.c
	@echo "* CC:  $<"
	@$(CC) $(CC_FLAGS) $< -o $@ -c

$(KERNEL_BUILD): $(ASM_OBJ) $(C_OBJ)
	@echo "* LD"
	@$(LD) -o $@ $^ $(LD_FLAGS)

format:
	@find $(SRC) -regex '.*\.\(c\|h\)' ! -name "*limine*" ! -name "*printf*" -exec $(FORMAT) -style=file -i {} \;

clean:
	rm -rf $(BUILD)
	rm -rf $(KERNEL_BUILD)