#include <iostream>
#include <iomanip>
#include <algorithm>
#include <deque>
#include <cmath>

#include "process.h"
#include "scheduler.h"

void Scheduler::addProcess(PCB* process) {
    if (process->state == ProcessState::READY) {
        ready_queue.push_back(process);
    }
}

void Scheduler::clearProcess() {
    ready_queue.clear();
    gantt.clear();
}

void Scheduler::printGantt() const {
    if (gantt.empty()) {
        std::cout << "尚无调度数据" << std::endl;
        return;
    }
    int i = 0;
    struct gantt_info_t {
        std::string name;
        int start;
        int end;
    };
    std::vector<gantt_info_t> gantt_info;
    while (i < gantt.size()) {
        std::string name = gantt[i];
        int start = i;
        while (i < gantt.size() && gantt[i] == name) { i ++; }
        int end = i;
        gantt_info.push_back({name, start, end});
    }
    for (const auto& info : gantt_info) {
        std::cout << info.name << "(" << info.start << "-" << info.end << ") ";
    }
    std::cout << std::endl;
    for (const auto& info : gantt_info) {
        std::cout << info.start;
        int width = info.name.size() + 1 
                    + std::to_string(info.start).size() + 1 
                    + std::to_string(info.end).size() + 2;
        std::cout << std::string(width - std::to_string(info.start).size(), ' ');
    }
    std::cout << gantt_info.back().end << std::endl;
}

void Scheduler::printStats() const {
    if (ready_queue.empty()) {
        std::cout << "尚无调度数据" << std::endl;
        return;
    }
    // 表头
    std::cout << "\n=== 调度统计 ===\n";
    std::cout << std::left
              << std::setw(12) << "进程"
              << std::setw(8)  << "到达"
              << std::setw(8)  << "执行"
              << std::setw(8)  << "完成"
              << std::setw(8)  << "周转"
              << std::setw(8)  << "等待"
              << "\n";

    double total_turnaround = 0;
    double total_waiting    = 0;

    for (const auto& p : ready_queue) {
        int turnaround = p->finish_time - p->arrival_time;
        int waiting    = turnaround - p->burst_time;
        total_turnaround += turnaround;
        total_waiting    += waiting;

        std::cout << std::left
                  << std::setw(12) << p->name
                  << std::setw(8)  << p->arrival_time
                  << std::setw(8)  << p->burst_time
                  << std::setw(8)  << p->finish_time
                  << std::setw(8)  << turnaround
                  << std::setw(8)  << waiting
                  << "\n";
    }

    int n = ready_queue.size();
    std::cout << std::fixed << std::setprecision(2)
              << "\n平均周转时间：" << total_turnaround / n
              << "   平均等待时间：" << total_waiting / n
              << "\n";
}

void Scheduler::run(const std::string& algorithm, MemoryManager& mem, int time_quantum) {
    std::sort(ready_queue.begin(), ready_queue.end(), [](const PCB* a, const PCB* b) {
        if (a->arrival_time == b->arrival_time) { return a->pid < b->pid; }
        return a->arrival_time < b->arrival_time;
    });
    if (algorithm == "FCFS") {
        int current_time = 0;
        for (const auto& process : ready_queue) {
            while (process->arrival_time > current_time) {
                gantt.push_back("idle");
                current_time ++;
            }
            process->dispatch();
            for (int i = 0; i < process->burst_time; i++) {
                gantt.push_back(process->name);
                current_time ++;
            }
            process->terminate(mem, current_time);
        }
    } else if (algorithm == "SJF") {
        int current_time = 0;
        std::vector<PCB*> ready_queue = this->ready_queue;
        struct CmpBurst {
            bool operator()(const PCB* a, const PCB* b) const {
                if (a->burst_time == b->burst_time) return a->pid > b->pid;
                return a->burst_time > b->burst_time;  // burst 小的在堆顶
            }
        };
        std::priority_queue<PCB*, std::vector<PCB*>, CmpBurst> pqueue;
        while (!ready_queue.empty() || !pqueue.empty()) {
            // ① 入队所有已到达进程
            while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                pqueue.push(ready_queue.front());
                ready_queue.erase(ready_queue.begin());
            }
            // ② pqueue 仍空 → CPU 空闲
            if (pqueue.empty()) {
                gantt.push_back("idle");
                current_time++;
                continue;
            }
            // ③ 取 burst 最短的进程，一次性跑完（非抢占）
            PCB* process = pqueue.top();
            pqueue.pop();
            process->dispatch();
            for (int i = 0; i < process->burst_time; i++) {
                gantt.push_back(process->name);
                current_time++;
                // 每个时间单位后检查新到达进程
                while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                    pqueue.push(ready_queue.front());
                    ready_queue.erase(ready_queue.begin());
                }
            }
            process->terminate(mem, current_time);
        }
    } else if (algorithm == "RR") {
        runRR(mem, time_quantum);
    } else if (algorithm == "Priority") {
        int current_time = 0;
        std::vector<PCB*> ready_queue = this->ready_queue;
        struct CmpBurst {
            bool operator()(const PCB* a, const PCB* b) const {
                if (a->priority == b->priority) return a->pid > b->pid;
                return a->priority > b->priority;  // burst 小的在堆顶
            }
        };
        std::priority_queue<PCB*, std::vector<PCB*>, CmpBurst> pqueue;
        while (!ready_queue.empty() || !pqueue.empty()) {
            // ① 入队所有已到达进程
            while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                pqueue.push(ready_queue.front());
                ready_queue.erase(ready_queue.begin());
            }
            // ② pqueue 仍空 → CPU 空闲
            if (pqueue.empty()) {
                gantt.push_back("idle");
                current_time++;
                continue;
            }
            // ③ 取 burst 最短的进程，一次性跑完（非抢占）
            PCB* process = pqueue.top();
            pqueue.pop();
            process->dispatch();
            for (int i = 0; i < process->burst_time; i++) {
                gantt.push_back(process->name);
                current_time++;
                // 每个时间单位后检查新到达进程
                while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                    pqueue.push(ready_queue.front());
                    ready_queue.erase(ready_queue.begin());
                }
            }
            process->terminate(mem, current_time);
        }
    }
}

void Scheduler::runRR(MemoryManager& mem, int time_quantum) {
    std::vector<PCB*> ready_queue = this->ready_queue;
    std::deque<PCB*> dqueue;
    int current_time = 0;
    while (!ready_queue.empty() || !dqueue.empty()) {
        if (dqueue.empty() && !ready_queue.empty()) {
            if (ready_queue.front()->arrival_time > current_time) {
                gantt.push_back("idle");
                current_time ++;
                continue;
            } else {
                while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                    dqueue.push_back(ready_queue.front());
                    ready_queue.erase(ready_queue.begin());
                }
            }
        }
        PCB* p = dqueue.front();
        int true_run_time = fmin(p->remaining_time, time_quantum);
        p->dispatch();
        while (true_run_time > 0) {
            gantt.push_back(p->name);
            current_time ++;
            true_run_time --;
            p->remaining_time --;
            while (!ready_queue.empty() && ready_queue.front()->arrival_time <= current_time) {
                dqueue.push_back(ready_queue.front());
                ready_queue.erase(ready_queue.begin());
            }
        }
        if (p->remaining_time <= 0) {
            p->terminate(mem, current_time);
            dqueue.pop_front();
        } else {
            p->preempt();
            dqueue.push_back(p);
            dqueue.pop_front();
        }
    }
}