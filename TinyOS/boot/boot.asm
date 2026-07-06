[bits 16]
[org 0x7C00]

start:
    ; 关闭中断, 初始化段寄存器（实模式下必须做）
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00    ; 栈从 0x7C00 向下增长

    ; 打印 "Loading..."
    sti
    mov si, loading_msg
    call print16
    cli

    ; 读取内核（从第2扇区开始，读取5个扇区到 0x1000:0x0000）
    mov ax, 0x1000
    mov es, ax          ; 目标段地址
    mov bx, 0           ; 目标偏移
    mov ah, 0x02        ; 功能：读扇区
    mov al, 5           ; 读5个扇区
    mov ch, 0           ; 柱面0
    mov cl, 2           ; 从第2扇区开始（第1扇区是引导扇区）
    mov dh, 0           ; 磁头0
    mov dl, 0x00        ; 软盘驱动器0
    int 0x13
    jc disk_error       ; CF=1 表示读取失败

    ; 加载 GDT
    lgdt [gdt_descriptor]

    ; 开启保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 远跳转到 32 位代码（刷新流水线，切换到保护模式代码段）
    ; 0x08 是代码段选择子（GDT 第1项，每项8字节，所以偏移=8=0x08）
    jmp 0x08:protected_mode_start

disk_error:
    mov si, error_msg
    call print16
    jmp $

; ── GDT 定义 ──────────────────────────────────────────────────
gdt_start:
    ; 第 0 项，空描述符（ 8 个 0 字节）
    dq 0

    ; 第1项：代码段（基址=0，限长=4GB，可执行可读，DPL=0）
    ; 格式复杂，按位拼接，直接用这个固定值：
    dw 0xFFFF    ; 段限长低16位
    dw 0x0000    ; 基址低16位
    db 0x00      ; 基址中8位
    db 10011010b ; 访问权限（代码段，Ring 0，可读可执行）
    db 11001111b ; 标志 + 段限长高4位（4KB粒度，32位，限长=0xFFFFF）
    db 0x00      ; 基址高8位

    ; 第2项：数据段（基址=0，限长=4GB，可读可写，DPL=0）
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b ; 访问权限（数据段，Ring 0，可读可写）
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; GDT 大小（字节数-1）
    dd gdt_start                 ; GDT 物理地址

; ── 32位保护模式代码 ────────────────────────────────────────────
[bits 32]
protected_mode_start:
    ; 重置数据段（选择子 0x10 = GDT 第2项）
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 设置栈（在 1MB 以下的安全位置）
    mov esp, 0x90000

    ; 跳转到 C 内核（地址 0x10000）
    jmp 0x08:0x10000

    jmp $

; ── 16位辅助函数 ────────────────────────────────────────────────
[bits 16]
print16:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp print16

.done:
    ret

; 字符串，\r\n，结束符0
loading_msg db 'Switching to protected mode...', 0x0D, 0x0A, 0
error_msg   db 'Disk read error!', 0

times 510-($-$$) db 0  ; 填充到 510 字节
dw 0xAA55              ; 引导签名