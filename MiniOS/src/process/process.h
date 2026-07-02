#pragma once

#include <string>
#include <iostream>
#include <iomanip>

#include "../memory/memory_manager.h"

enum class ProcessState {
    NEW, READY, RUNNING, BLOCKED, TERMINATED
};

// 将状态转为可打印字符串的辅助函数
inline std::string stateToString(ProcessState s) {
    switch (s) {
        case ProcessState::NEW: return "NEW";
        case ProcessState::READY: return "READY";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::BLOCKED: return "BLOCKED";
        case ProcessState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// PCB 结构体
struct PCB {
    // 上表中所有字段
    int pid;
    std::string name;
    ProcessState state;
    int priority;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int finish_time;
    int memory_size;
    int memory_base;

    // 构造函数：接收 pid, name, priority, arrival_time, burst_time, memory_size
    // 在构造函数中：
    //   state = ProcessState::NEW
    //   remaining_time = burst_time
    //   memory_base = -1（-1 表示还未分配内存）
    PCB(int pid, std::string name, int priority, int arrival_time, int burst_time, int memory_size) {
        this->pid = pid;
        this->name = name;
        this->priority = priority;
        this->arrival_time = arrival_time;
        this->burst_time = burst_time;
        this->memory_size = memory_size;
        this->state = ProcessState::NEW;
        this->remaining_time = burst_time;
        this->finish_time = -1;
        this->memory_base = -1;
    }

    // NEW → READY（进程就绪）
    void admit(MemoryManager& mem) {
        if (this->state == ProcessState::NEW) {
            int addr = mem.allocate(pid, name, memory_size);
            if (addr == -1) {
                std::cout << "Memory allocation failed for process " << name << std::endl;
                return;
            }
            memory_base = addr;            
            this->state = ProcessState::READY;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → READY" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "NEW → READY, " 
            << "内存: " 
            << std::setw(4) << memory_base 
            << "~" 
            << std::setw(4) << (memory_base + memory_size - 1)
            << "KB"
            << std::endl;
    }      
    // READY → RUNNING（被调度器选中）
    void dispatch() {
        if (this->state == ProcessState::READY) {
            this->state = ProcessState::RUNNING;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → RUNNING" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "READY → RUNNING"
            << std::endl;
    }
    // RUNNING → READY（被抢占，时间片到）
    void preempt() {
        if (this->state == ProcessState::RUNNING) {
            this->state = ProcessState::READY;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → READY" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "RUNNING → READY"
            << std::endl;
    }
    // RUNNING → BLOCKED（等待事件）
    void block() {
        if (this->state == ProcessState::RUNNING) {
            this->state = ProcessState::BLOCKED;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → BLOCKED" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "RUNNING → BLOCKED"
            << std::endl;
    }
    // BLOCKED → READY（事件完成，唤醒）
    void wakeup() {
        if (this->state == ProcessState::BLOCKED) {
            this->state = ProcessState::READY;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → READY" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "BLOCKED → READY"
            << std::endl;
    }
    // RUNNING → TERMINATED（执行完毕）
    void terminate(MemoryManager& mem, int current_time) {
        if (this->state == ProcessState::RUNNING) {
            mem.free(this->pid);
            this->state = ProcessState::TERMINATED;
            this->finish_time = current_time;
        } else {
            std::cout << "Invalid state transition: " << stateToString(this->state) << " → TERMINATED" << std::endl;
            return;
        }
        std::cout
            << "[PID=" << std::right << std::setw(2) << this->pid << "] "
            << std::left  << std::setw(16) << this->name
            << "RUNNING → TERMINATED, "
            << "释放内存: " 
            << std::setw(4) << memory_base 
            << "~" 
            << std::setw(4) << (memory_base + memory_size - 1)
            << "KB"
            << std::endl;
    }
};

// 打印单个 PCB 信息的辅助函数
inline void printPCB(const PCB& p) {
    // 使用 setw + left 保证每列固定宽度，字段之间上下对齐
    // 示例输出：[PID= 1] init            状态:READY     优先级: 1  到达: 0  执行: 5  内存: 100KB  基址:  0
    std::cout
        << "[PID=" << std::right << std::setw(2) << p.pid << "] "
        << std::left  << std::setw(10) << p.name
        << std::left  << "状态:" << std::setw(12) << stateToString(p.state)
        << std::right << "优先级:" << std::setw(2) << p.priority
        << "  到达:" << std::setw(3) << p.arrival_time
        << "  执行:" << std::setw(3) << p.burst_time
        << "  内存:" << std::setw(5) << p.memory_size << "KB"
        << "  基址:" << std::setw(4) << p.memory_base
        << std::endl;
}