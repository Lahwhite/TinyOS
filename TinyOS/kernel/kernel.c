#include "../drivers/vga.h"

void kernel_main() {
    vga_init();
    vga_clear();
    vga_puts("Hello, TinyOS!\n");
    vga_puts("Kernel is running in 32-bit protected mode.\n");
    
    while (1) {}
}