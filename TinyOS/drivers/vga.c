#include "vga.h"

static char* vga_buf = (char*)0xB8000;  // 显存指针
static int cursor_x = 0;                // 当前列（0~79）
static int cursor_y = 0;                // 当前行（0~24）
static char color = 0x0F;               // 当前颜色（黑底白字）


void vga_init() {
    vga_clear();
}

void vga_putchar(char c) {
    // 1. 处理字符
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int offset = (cursor_y * 80 + cursor_x) * 2;
        vga_buf[offset]     = c;
        vga_buf[offset + 1] = color;
        cursor_x++;
        // 行满立即换行，避免留到下次调用与 \n 叠加
        if (cursor_x == 80) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    // 2. 统一滚屏判断
    if (cursor_y == 25) {
        // 把第 1~24 行上移到第 0~23 行
        for (int row = 0; row < 24; row++) {
            for (int col = 0; col < 80; col++) {
                int dst = (row * 80 + col) * 2;
                int src = ((row + 1) * 80 + col) * 2;
                vga_buf[dst]     = vga_buf[src];
                vga_buf[dst + 1] = vga_buf[src + 1];
            }
        }
        // 清空最后一行
        for (int col = 0; col < 80; col++) {
            int offset = (24 * 80 + col) * 2;
            vga_buf[offset]     = ' ';
            vga_buf[offset + 1] = color;
        }
        cursor_y = 24;
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

void vga_clear() {
    for (int i = 0; i < 80 * 25; i ++) {
        vga_buf[i * 2] = ' ';
        vga_buf[i * 2 + 1] = color;
    }
    cursor_x = 0;
    cursor_y = 0;
}