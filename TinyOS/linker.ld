ENTRY(_start)

SECTIONS {
    . = 0x10000;        /* 内核加载到物理地址 0x10000 */
    
    .text : {
        *(.text)        /* 所有代码段 */
    }
    
    .rodata : {
        *(.rodata)      /* 只读数据（字符串常量等） */
    }
    
    .data : {
        *(.data)        /* 已初始化数据 */
    }
    
    .bss : {
        *(.bss)         /* 未初始化数据（清零） */
    }
}