#include "string.h"

// ══════════════════════════════════════════════
//  内存操作
// ══════════════════════════════════════════════

void* memset(void* dest, int c, size_t n) {
    unsigned char* p = (unsigned char*)dest;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char*       d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* pa = (const unsigned char*)a;
    const unsigned char* pb = (const unsigned char*)b;
    while (n--) {
        if (*pa != *pb) {
            return (int)*pa - (int)*pb;
        }
        pa++;
        pb++;
    }
    return 0;
}

// ══════════════════════════════════════════════
//  字符串操作
// ══════════════════════════════════════════════

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++)) {}
    return ret;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n > 0 && *src) {
        *dest++ = *src++;
        n--;
    }
    // 剩余位置补 '\0'
    while (n-- > 0) {
        *dest++ = '\0';
    }
    return ret;
}

char* strcat(char* dest, const char* src) {
    char* ret = dest;
    // 移动到 dest 末尾
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++)) {}
    return ret;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while (n-- > 0 && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
    while (n-- > 0) {
        if (*a != *b) {
            return (int)(unsigned char)*a - (int)(unsigned char)*b;
        }
        if (*a == '\0') {
            return 0;
        }
        a++;
        b++;
    }
    return 0;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    // 检查终止符本身
    if ((char)c == '\0') {
        return (char*)str;
    }
    return 0;
}

// ══════════════════════════════════════════════
//  数值转换
// ══════════════════════════════════════════════

char* itoa(int value, char* buf) {
    char  tmp[12];  // 最长 "-2147483648\0"
    char* p   = tmp;
    char* out = buf;
    int   neg = 0;

    if (value < 0) {
        neg   = 1;
        value = -value;
    }

    // 处理 0
    if (value == 0) {
        *p++ = '0';
    } else {
        while (value > 0) {
            *p++ = '0' + (value % 10);
            value /= 10;
        }
    }

    if (neg) {
        *out++ = '-';
    }

    // 逆序写入 buf
    while (p > tmp) {
        *out++ = *--p;
    }
    *out = '\0';
    return buf;
}

char* itohex(unsigned int value, char* buf) {
    static const char digits[] = "0123456789ABCDEF";
    char  tmp[10];  // 最长 8 位十六进制
    char* p   = tmp;
    char* out = buf;

    // 写 "0x" 前缀
    *out++ = '0';
    *out++ = 'x';

    if (value == 0) {
        *p++ = '0';
    } else {
        while (value > 0) {
            *p++ = digits[value & 0xF];
            value >>= 4;
        }
    }

    // 逆序写入
    while (p > tmp) {
        *out++ = *--p;
    }
    *out = '\0';
    return buf;
}
