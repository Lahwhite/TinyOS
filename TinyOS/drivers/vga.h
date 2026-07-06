#pragma once

// 颜色定义
#define VGA_COLOR_BLACK   0
#define VGA_COLOR_GREEN   2
#define VGA_COLOR_RED     4
#define VGA_COLOR_WHITE   15

// 组合前景色和背景色
#define VGA_COLOR(fg, bg) ((bg << 4) | fg)

void vga_init();
void vga_putchar(char c);
void vga_puts(const char* str);
void vga_clear();