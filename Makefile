# 🧠 NEOOS MAKEFILE — INSANO E OTIMIZADO 🧠

# Compiler and assembler
CC     = clang
AS     = nasm
LD     = ld.lld
CFLAGS = --target=x86_64-elf -ffreestanding -m64 -nostdlib -nostdinc -fno-builtin
ASFLAGS = -f bin

# Paths
BUILD   = build
SRC     = kernel
BOOT    = boot
OUT_IMG = neoos.img
KERNEL_BIN = kernel.bin

# Sources
KERNEL_CSRC = $(wildcard $(SRC)/*.c)
KERNEL_OBJS = $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(KERNEL_CSRC))
BOOT1 = $(BOOT)/boot1.asm
BOOT2 = $(BOOT)/boot2.asm

# Targets
all: $(OUT_IMG)

# Step 1: Compile C files to .o
$(BUILD)/%.o: $(SRC)/%.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Step 2: Link all .o files into kernel.bin
$(KERNEL_BIN): $(KERNEL_OBJS)
	$(LD) -nostdlib -Ttext 0x1000 -o $@ $^

# Step 3: Assemble bootloaders
boot1.bin: $(BOOT1)
	$(AS) $(ASFLAGS) $< -o $@

boot2.bin: $(BOOT2)
	$(AS) $(ASFLAGS) $< -o $@

# Step 4: Build floppy image (1.44MB)
$(OUT_IMG): boot1.bin boot2.bin $(KERNEL_BIN)
	dd if=/dev/zero of=$(OUT_IMG) bs=512 count=2880
	dd if=boot1.bin of=$(OUT_IMG) conv=notrunc
	dd if=boot2.bin of=$(OUT_IMG) seek=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(OUT_IMG) seek=10 conv=notrunc

# Clean everything
clean:
	rm -rf $(BUILD) boot1.bin boot2.bin $(KERNEL_BIN) $(OUT_IMG)

.PHONY: all clean
