#pragma once

// 裸机环境下不能使用标准库头文件，自行定义 size_t
typedef unsigned int size_t;

// ──────────────────────────────────────────────
//  内存操作
// ──────────────────────────────────────────────

// 将 dest 起始的 n 个字节设为 c
void* memset(void* dest, int c, size_t n);

// 将 src 起始的 n 个字节复制到 dest
void* memcpy(void* dest, const void* src, size_t n);

// 比较两段内存的前 n 个字节，返回 <0 / 0 / >0
int   memcmp(const void* a, const void* b, size_t n);

// ──────────────────────────────────────────────
//  字符串操作
// ──────────────────────────────────────────────

// 返回字符串长度（不含终止符 '\0'）
size_t strlen(const char* str);

// 将 src 复制到 dest，返回 dest
char*  strcpy(char* dest, const char* src);

// 将 src 的前 n 个字符复制到 dest，不足则补 '\0'
char*  strncpy(char* dest, const char* src, size_t n);

// 将 src 追加到 dest 末尾，返回 dest
char*  strcat(char* dest, const char* src);

// 将 src 的前 n 个字符追加到 dest 末尾，返回 dest
char*  strncat(char* dest, const char* src, size_t n);

// 比较两个字符串，返回 <0 / 0 / >0
int    strcmp(const char* a, const char* b);

// 比较两个字符串的前 n 个字节，返回 <0 / 0 / >0
int    strncmp(const char* a, const char* b, size_t n);

// 查找字符 c 在字符串 str 中首次出现的位置，未找到返回 NULL
char*  strchr(const char* str, int c);

// 将整数转换为字符串（十进制，带符号），返回 buf
char*  itoa(int value, char* buf);

// 将无符号整数转换为十六进制字符串（如 "0x1F"），返回 buf
char*  itohex(unsigned int value, char* buf);
