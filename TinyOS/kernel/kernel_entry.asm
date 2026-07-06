[bits 32]
[extern kernel_main]   ; 声明 kernel_main 是外部 C 函数

global _start
_start:
    call kernel_main
    jmp $