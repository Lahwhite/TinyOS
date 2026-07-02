#include <iostream>
#include <vector>
#include <thread>

#include "process/process.h"
#include "memory/memory_manager.h"
#include "process/scheduler.h"
#include "sync/semaphore.h"
#include "sync/producer_consumer.h"
#include "filesystem/filesystem.h"

Semaphore sem(1);

void increment(int& counter, int times, Semaphore& sem) {
    for (int i = 0; i < times; i++) {
        sem.acquire();
        counter++;
        sem.release();
    }
}

int main() {
    std::cout << "MiniOS 启动...\n\n";

    MemoryManager memory_manager(1024);
    // 手动创建几个进程（第三阶段前不需要自动化）
    // 参数：pid, name, priority, arrival_time, burst_time, memory_size(KB)
    std::vector<PCB> processes = {
        PCB(1, "init",   1, 0, 5, 100),
        PCB(2, "shell",  2, 0, 3, 200),
        PCB(3, "vim",    3, 1, 8, 150),
        PCB(4, "gcc",    2, 2, 6, 300),
        PCB(5, "python", 3, 3, 4,  80),
    };
    std::cout << "=== 进程创建 ===\n";
    for (auto& p : processes) {
        p.admit(memory_manager);
        printPCB(p);
    }

    std::cout << "=== 调度算法 ===\n";
    std::vector<PCB> copy_processes = processes;
    Scheduler scheduler;

    copy_processes = processes;
    scheduler.clearProcess();
    for (auto& p : copy_processes) { scheduler.addProcess(&p); }
    scheduler.run("Priority", memory_manager, 5);
    scheduler.printGantt();
    scheduler.printStats();

    memory_manager.printLayout();
    memory_manager.printStats();

    // runProducerConsumer();

    std::cout << "=== 文件管理 ===\n";
    FileSystem fs;

    fs.mkdir("home");
    fs.mkdir("etc");
    fs.cd("home");
    fs.mkdir("user");
    fs.touch("readme.txt");
    fs.cdUp();

    // 模拟进程执行：进程执行完后写一个日志文件
    fs.cd("home");
    fs.mkdir("logs");
    fs.cd("logs");

    // P1 完成后写日志
    fs.touch("p1.log");
    fs.write("p1.log", "[P1 init] 进程执行完毕，退出码 0");

    // P2 完成后写日志
    fs.touch("p2.log");
    fs.write("p2.log", "[P2 shell] 进程执行完毕，退出码 0");

    // 读回日志
    std::string log = fs.read("p1.log");
    std::cout << "读取日志：" << log << "\n";

    fs.cdUp(); fs.cdUp();
    fs.printTree();

    return 0;
}