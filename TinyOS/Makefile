CC      = i686-elf-gcc
CXX     = i686-elf-g++
LD      = i686-elf-ld
NASM    = nasm
QEMU    = qemu-system-i386

# GCC 编译选项（裸机，不链接任何库）
CFLAGS  = -m32 -ffreestanding -fno-stack-protector -nostdlib -nostdinc -O2
# G++ 编译选项（额外禁用 RTTI 和异常，适合裸机）
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions

all: tinyos.img

boot/boot.bin: boot/boot.asm
	$(NASM) -f bin $< -o $@

kernel/kernel_entry.o: kernel/kernel_entry.asm
	$(NASM) -f elf32 $< -o $@

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

drivers/vga.o: drivers/vga.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/string.o: lib/string.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

kernel/kernel.elf: kernel/kernel_entry.o kernel/kernel.o drivers/vga.o lib/string.o
	$(LD) -T linker.ld -o $@ $^

kernel/kernel.bin: kernel/kernel.elf
	i686-elf-objcopy -O binary $< $@

tinyos.img: boot/boot.bin kernel/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=boot/boot.bin of=$@ conv=notrunc
	dd if=kernel/kernel.bin of=$@ bs=512 seek=1 conv=notrunc

run: tinyos.img
	$(QEMU) -drive format=raw,if=floppy,file=$<

clean:
	rm -f boot/*.bin kernel/*.o kernel/*.bin kernel/*.elf drivers/*.o lib/*.o tinyos.img