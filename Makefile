# ===== Configurações Infalíveis =====
ASM      := nasm
CC       := clang
LD       := ld.lld
OBJCOPY  := llvm-objcopy
XORRISO  := xorriso

# Flags do compilador
CFLAGS   := --target=x86_64-elf -ffreestanding -fno-stack-protector \
            -fno-pic -mno-red-zone -mno-sse -mno-sse2 \
            -Wall -Wextra -Werror -Ikernel/ -O2

# Flags do linker
LDFLAGS  := -nostdlib -static -Ttext=0x8000 -z max-page-size=0x1000

# ===== Arquivos =====
KERNEL_SRCS := $(wildcard kernel/*.c)
KERNEL_OBJS := $(patsubst %.c,%.o,$(KERNEL_SRCS))
KERNEL      := kernel.bin
BOOT1       := boot1.bin
BOOT2       := boot2.bin
DISK_IMG    := neoos.img
ISO_IMG     := neoos.iso

# ===== Regras Principais =====
all: $(ISO_IMG)  # Build completo (agora gera ISO diretamente)

$(ISO_IMG): $(DISK_IMG)
	@echo "📀 Criando ISO com xorriso..."
	@$(XORRISO) -as mkisofs \
		-b $(DISK_IMG) \
		-no-emul-boot \
		-boot-load-size 4 \
		-iso-level 2 \
		-full-iso9660-filenames \
		-o $(ISO_IMG) \
		$(DISK_IMG) >/dev/null 2>&1
	@echo "✅ ISO gerada: $(ISO_IMG)"

$(DISK_IMG): $(BOOT1) $(BOOT2) $(KERNEL)
	@echo "🛠️  Montando imagem de disco..."
	@dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	@dd if=$(BOOT1) of=$@ conv=notrunc status=none
	@dd if=$(BOOT2) of=$@ seek=1 conv=notrunc status=none
	@dd if=$(KERNEL) of=$@ seek=2 conv=notrunc status=none

# ===== Bootloader e Kernel =====
$(BOOT1): boot/boot1.asm
	@echo "🔧 Compilando Bootloader (Stage 1)..."
	@$(ASM) -f bin $< -o $@ -Wall

$(BOOT2): boot/boot2.asm
	@echo "🔧 Compilando Bootloader (Stage 2)..."
	@$(ASM) -f bin $< -o $@ -Wall

kernel/%.o: kernel/%.c
	@echo "🔄 Compilando $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_OBJS)
	@echo "🧩 Linkando Kernel..."
	@$(LD) $(LDFLAGS) $^ -o $@
	@$(OBJCOPY) -O binary $@ $@

# ===== Utilitários =====
run: $(ISO_IMG)
	@echo "🚀 Iniciando QEMU (from ISO)..."
	@qemu-system-x86_64 -cdrom $(ISO_IMG) -d guest_errors

debug: $(ISO_IMG)
	@qemu-system-x86_64 -cdrom $(ISO_IMG) -S -s &
	@echo "🐛 Debug mode: conecte com 'gdb -ex 'target remote localhost:1234'"

clean:
	@echo "🧹 Limpando build..."
	@rm -f $(BOOT1) $(BOOT2) $(KERNEL) $(KERNEL_OBJS) $(DISK_IMG) $(ISO_IMG)

.PHONY: all clean run debug
