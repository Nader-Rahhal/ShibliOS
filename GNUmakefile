.SUFFIXES:

override OUTPUT := shiblios

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    CC := clang
    LD := ld.lld
    NASM := nasm
else
    CC := cc
    LD := ld
    NASM := nasm
endif


CFLAGS := -g -O2 -pipe
CPPFLAGS :=
NASMFLAGS := -g
LDFLAGS :=

CC_IS_CLANG := $(shell $(CC) --version 2>/dev/null | grep -q clang && echo 1 || echo 0)

ifeq ($(CC_IS_CLANG),1)
    CFLAGS += -target x86_64-unknown-none-elf
endif

CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel

CPPFLAGS += \
    -I src \
    -I include \
    -MMD \
    -MP

NASMFLAGS += \
    -f elf64 \
    -Wall

LDFLAGS += \
    -nostdlib \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker.lds

ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -m elf_x86_64
else
    LDFLAGS += -static
endif


SRCFILES := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
CFILES := $(filter %.c,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))
NASMFILES := $(filter %.asm,$(SRCFILES))

OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

.PHONY: all
all: bin/$(OUTPUT)

-include $(HEADER_DEPS)

bin/$(OUTPUT): GNUmakefile linker.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

obj/%.c.o: %.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj/%.S.o: %.S GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj/%.asm.o: %.asm GNUmakefile
	mkdir -p "$(dir $@)"
	$(NASM) $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf bin obj iso_root image.iso

# Build ISO with Limine
.PHONY: build
build:
	git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1 || true
	make -C limine
	mkdir -p iso_root/boot

	cp -v bin/$(OUTPUT) iso_root/boot/
	mkdir -p iso_root/boot/limine

	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v font.psf iso_root/boot/

	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/

	xorriso -as mkisofs -R -r -J \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o image.iso

	./limine/limine bios-install image.iso

.PHONY: format
format:
	qemu-img create -f raw disk.img 100M
	/opt/homebrew/opt/e2fsprogs/sbin/mkfs.ext2 disk.img
# Run in QEMU
.PHONY: run
run:
	qemu-system-x86_64 \
    -m 512M \
    -cdrom image.iso \
    -drive file=disk.img,format=raw,if=ide \
    -serial stdio