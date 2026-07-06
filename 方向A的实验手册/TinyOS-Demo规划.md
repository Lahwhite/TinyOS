# TinyOS Demo 规划（方向 A：真实运行在硬件上）

## 项目目标

用 C + 少量 x86 汇编，写一个能在 QEMU 模拟器上真实启动并运行的微型操作系统内核。

```
启动流程：
BIOS → 加载引导扇区 → 切换保护模式 → 进入 C 内核
→ 初始化中断 → 启动时钟 → 调度进程 → 进程能读写文件
```

**和 MiniOS 的本质区别：**
- MiniOS：运行在 macOS/Linux 之上，调度靠手动调用函数
- TinyOS：直接运行在 QEMU 模拟的 x86 硬件上，调度靠时钟中断强制触发

---

## 运行环境

| 工具 | 用途 |
|------|------|
| QEMU | x86 硬件模拟器，运行你写的内核 |
| NASM | 汇编器，编译 `.asm` 文件 |
| GCC（i686-elf 交叉编译） | 编译裸机 C 代码（不链接标准库） |
| GNU ld | 链接器，把各模块链接成内核二进制 |
| Make | 构建系统 |

---

## 项目结构

```
TinyOS/
├── Makefile
├── linker.ld                   # 链接脚本：规定内核加载到内存哪个地址
├── boot/
│   └── boot.asm                # 引导扇区（512字节，BIOS 第一个执行的代码）
├── kernel/
│   ├── kernel_entry.asm        # 从汇编跳转到 C 的入口
│   ├── kernel.c                # C 内核主函数
│   ├── gdt.h/c                 # 全局描述符表（保护模式基础）
│   ├── idt.h/c                 # 中断描述符表（注册中断处理函数）
│   ├── isr.asm                 # 中断服务例程（汇编部分）
│   ├── timer.h/c               # 时钟中断（驱动进程调度）
│   └── pic.h/c                 # 可编程中断控制器（管理硬件中断）
├── process/
│   ├── process.h/c             # PCB（从 MiniOS 移植，去掉 STL）
│   └── scheduler.h/c           # 调度器（从 MiniOS 移植算法）
├── memory/
│   ├── pmm.h/c                 # 物理内存管理（位图法）
│   ├── vmm.h/c                 # 虚拟内存管理（页表）
│   └── heap.h/c                # 内核堆（自己实现 kmalloc/kfree）
├── drivers/
│   ├── vga.h/c                 # VGA 文字模式驱动（直接写显存打印字符）
│   ├── keyboard.h/c            # 键盘驱动（读键盘中断）
│   └── ata.h/c                 # ATA 磁盘驱动（读写虚拟磁盘）
├── fs/
│   └── simplefs.h/c            # 简易文件系统（从 MiniOS 移植 inode 结构）
└── lib/
    ├── string.h/c              # 自己实现 memcpy/memset/strlen（不能用标准库）
    └── stdio.h/c               # 自己实现 kprintf（内核打印函数）
```

---

## 与 MiniOS 的对应关系

| MiniOS 文件 | TinyOS 对应 | 迁移难度 |
|------------|------------|---------|
| `process/process.h` | `process/process.h/c` | ⭐ 改掉 std::string，其余逻辑一样 |
| `process/scheduler.cpp` | `process/scheduler.h/c` | ⭐ 改掉 STL，算法完全一样 |
| `memory/memory_manager.cpp` | `memory/heap.h/c` | ⭐⭐ 改用自己的链表实现 |
| `memory/page_table.cpp` | `memory/vmm.h/c` | ⭐⭐ 逻辑一样，寄存器操作要用汇编 |
| `sync/semaphore.cpp` | 内核自旋锁 | ⭐⭐ 换成原子操作，不能用 condition_variable |
| `filesystem/filesystem.cpp` | `fs/simplefs.h/c` | ⭐⭐ 去掉 std::map，用数组存目录项 |
| `io/disk_scheduler.cpp` | 集成进 `drivers/ata.h/c` | ⭐⭐⭐ 需要了解 ATA I/O 端口协议 |
| **无对应** | `boot/boot.asm` | ⭐⭐⭐⭐ 全新，需要学 x86 汇编 |
| **无对应** | `kernel/idt.h/c` + `isr.asm` | ⭐⭐⭐⭐ 全新，中断机制 |
| **无对应** | `drivers/vga.h/c` | ⭐⭐ 直接写显存，相对简单 |

---

## 分阶段规划

### 第一阶段：引导，让 QEMU 启动你的代码

**目标：** QEMU 启动后，屏幕上打印 `Hello, TinyOS!`

**涉及：**
- `boot/boot.asm`：512 字节引导扇区，BIOS 把它加载到内存 0x7C00
- `drivers/vga.h/c`：直接写 VGA 显存（物理地址 0xB8000）打印字符
- 实模式 → 保护模式的切换（GDT 初始化）

**里程碑：** `qemu-system-i386 -drive format=raw,if=floppy,file=tinyos.img` 启动后看到文字

---

### 第二阶段：中断，让 OS 能响应事件

**目标：** 时钟每隔 1 秒打印一次计时，键盘按键后打印对应字符

**涉及：**
- `kernel/idt.h/c`：中断描述符表，注册 256 个中断处理函数
- `kernel/isr.asm`：保存寄存器、调用 C 处理函数、恢复寄存器
- `kernel/pic.h/c`：初始化 8259A 中断控制器，开启时钟和键盘中断
- `kernel/timer.h/c`：时钟中断处理函数

**里程碑：** 屏幕上每秒打印一个数字，按键后打印对应字母

---

### 第三阶段：内存管理，内核能动态分配内存

**目标：** 实现 `kmalloc` / `kfree`，内核代码能用动态内存

**涉及：**
- `memory/pmm.h/c`：物理内存管理，用位图记录哪些页帧空闲
- `memory/vmm.h/c`：页表初始化，开启分页模式（`cr0` 寄存器）
- `memory/heap.h/c`：基于物理内存管理实现的内核堆

**里程碑：** 能调用 `kmalloc(100)` 分配内存，`kfree` 归还

---

### 第四阶段：进程，真正的多任务切换

**目标：** 创建 3 个进程，时钟中断触发调度器切换，屏幕上能看到进程交替打印

**涉及：**
- `process/process.h/c`：PCB 结构（从 MiniOS 移植，替换 STL）
- 上下文切换（`isr.asm` 里保存/恢复全部寄存器 + 切换栈指针）
- `process/scheduler.h/c`：RR 调度（从 MiniOS 移植算法）
- 时钟中断处理函数里调用调度器

**里程碑：** 3 个进程分别打印 `A`、`B`、`C`，屏幕上交替出现

**这是 TinyOS 和 MiniOS 最大的区别：** MiniOS 的调度是你手动 `scheduler.run()` 触发的，TinyOS 的调度是时钟中断**强制**触发的，进程完全无感知。

---

### 第五阶段：磁盘驱动 + 文件系统

**目标：** 进程能读写 QEMU 模拟的虚拟磁盘，文件持久化

**涉及：**
- `drivers/ata.h/c`：ATA PIO 模式读写磁盘（通过 I/O 端口）
- `fs/simplefs.h/c`：inode 结构（从 MiniOS 移植，替换 std::map）
- 文件数据存在磁盘上，不是内存字符串

**里程碑：** 进程写 `hello.txt`，重启 QEMU 后还能读出来

---

### 第六阶段：用户态，保护模式真正发挥作用

**目标：** 内核运行在 Ring 0，用户进程运行在 Ring 3，用户态不能直接访问内核内存

**涉及：**
- GDT 新增用户态段描述符（Ring 3）
- 系统调用：用户态通过 `int 0x80` 进入内核态
- 实现 `sys_read`、`sys_write`、`sys_exit` 系统调用

**里程碑：** 用户态进程调用 `sys_write` 打印字符，直接访问内核内存会触发 General Protection Fault

---

## 学习前置知识

在开始写代码之前，需要先了解：

| 知识点 | 学习资源 | 重要程度 |
|-------|---------|---------|
| x86 实模式 / 保护模式 | OSDev Wiki - Protected Mode | ⭐⭐⭐⭐⭐ |
| GDT（全局描述符表） | OSDev Wiki - GDT | ⭐⭐⭐⭐⭐ |
| IDT + 中断处理 | OSDev Wiki - IDT | ⭐⭐⭐⭐⭐ |
| x86 汇编基础 | NASM 文档 / CS:APP 第三章 | ⭐⭐⭐⭐ |
| ELF 格式 + 链接脚本 | GNU ld 文档 | ⭐⭐⭐ |
| ATA PIO 磁盘协议 | OSDev Wiki - ATA PIO | ⭐⭐⭐ |

**推荐参考项目（按难度排序）：**
1. **xv6**（MIT）：最好的教学 OS，代码简洁，有配套课程
2. **OSDev Bare Bones**：OSDev Wiki 的最小引导教程，起点
3. **Writing a Simple Operating System from Scratch**（Nick Blundell）：免费 PDF，手把手

---

## 和 MiniOS 的完整对比

```
MiniOS（方向 B）                    TinyOS（方向 A）
─────────────────                  ─────────────────
运行在 macOS/Linux 上               直接运行在 QEMU x86 硬件上
main() 是入口                       引导扇区（0x7C00）是入口
std::vector / std::string           手写链表 / 字符数组
std::thread 模拟并发                时钟中断真实抢占
手动调用 scheduler.run()            中断自动触发调度
文件内容存字符串                     文件内容存磁盘扇区
没有权限区分                        Ring 0（内核）/ Ring 3（用户）
无需了解汇编                        需要写引导和中断处理汇编
从 MiniOS 开始，2-4 周能完成        独立项目，需要 2-3 个月
```

---

## 开始条件

完成 MiniOS 六个阶段后，再开始 TinyOS。理由：

- TinyOS 的调度、内存、文件算法和 MiniOS 完全一样，区别只在底层接口
- 在 MiniOS 里把算法想清楚，移植到 TinyOS 时只需要替换数据结构，不需要重新设计逻辑
- MiniOS 的 C++ 代码是 TinyOS C 代码的"设计草稿"
