#pragma once

#include <string>
#include <vector>

#include "process.h"
#include "../memory/memory_manager.h"

class Scheduler {
public:
    // 将进程加入就绪队列（进程状态必须是 READY）
    void addProcess(PCB* p);
    void clearProcess();

    // 运行调度：选择算法，模拟所有进程执行完毕
    // algorithm: "FCFS" / "SJF" / "RR" / "Priority"
    // time_quantum: RR 算法的时间片大小（其他算法忽略此参数）
    void run(const std::string& algorithm, MemoryManager& mem, int time_quantum = 2);

    // 打印 Gantt 图（调用 run 后才有数据）
    void printGantt() const;

    // 打印各进程的调度统计信息
    void printStats() const;

private:
    std::vector<PCB*> ready_queue;   // 就绪队列，存指针（不拷贝 PCB）

    // Gantt 记录：每个时间单位运行的进程名
    std::vector<std::string> gantt;  // gantt[t] = 时刻 t 运行的进程名，"idle" 表示 CPU 空闲

    void runRR(MemoryManager& mem, int time_quantum);
};