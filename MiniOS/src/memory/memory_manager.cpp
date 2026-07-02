#include <iostream>
#include <iomanip>

#include "memory_manager.h"

MemoryManager::MemoryManager(int total_kb, AllocPolicy policy) {
    total_size = total_kb;
    this->policy = policy;

    MemBlock memblock;
    memblock.start_address = 0;
    memblock.size = total_kb;
    memblock.pid = -1;
    memblock.process_name = "FREE";

    blocks.push_back(memblock);
}

void MemoryManager::printLayout() const {
    std::cout << "=== 内存布局（" << total_size << "KB）===" << std::endl;

    for (const auto& block : blocks) {
        int end = block.start_address + block.size - 1;

        std::cout
            << "["
            << std::right << std::setw(4) << block.start_address
            << " - "
            << std::right << std::setw(4) << end
            << "] "
            << std::right << std::setw(4) << block.size << "KB  ";

        if (block.pid == -1) { std::cout << "FREE"; } 
        else { std::cout << "P" << block.pid << "(" << block.process_name << ")"; }

        std::cout << std::endl;
    }
}

void MemoryManager::printStats() const {
    std::cout << "=== 内存统计 ===" << std::endl;
    int used = 0;
    for (const auto& block : blocks) {
        if (block.pid != -1) {  // 只统计已占用块，FREE 块不计入
            used += block.size;
        }
    }
    std::cout << "已用: " << used << "KB" << "/" << total_size << "KB" 
    << "(" << std::fixed << std::setprecision(2) << (static_cast<double>(used) / total_size * 100) << "%)  " 
    << "空闲: " << total_size - used << "KB" << std::endl;
}

// 统一分配入口：按 policy 分派到具体算法
int MemoryManager::allocate(int pid, const std::string& name, int size) {
    switch (policy) {
        case AllocPolicy::BEST_FIT:  return bestFit(pid, name, size);
        case AllocPolicy::FIRST_FIT: return firstFit(pid, name, size);
        default:                     return firstFit(pid, name, size);
    }
}

int MemoryManager::firstFit(int pid, const std::string& name, int size) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->pid == -1 && it->size >= size) {
            int base = it->start_address;

            // 若有剩余空间，切割出一块新的 FREE 块，插到当前块正后面
            int remaining = it->size - size;
            if (remaining > 0) {
                MemBlock remaining_block;
                remaining_block.start_address = base + size;
                remaining_block.size = remaining;
                remaining_block.pid = -1;
                remaining_block.process_name = "FREE";
                // insert(pos, val)：插到 pos 之前，即当前块的紧邻后方
                blocks.insert(std::next(it), remaining_block);
            }

            // 当前块占用
            it->pid = pid;
            it->process_name = name;
            it->size = size;

            return base;
        }
    }
    return -1;
}

void MemoryManager::free(int pid) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->pid == pid) {
            it->pid = -1;
            it->process_name = "FREE";
            break;  // 找到即释放，退出循环，再合并，避免迭代器失效
        }
    }
    mergeAdjacentFree();  // 循环结束后再合并，此时迭代器已不再被持有
}

void MemoryManager::mergeAdjacentFree() {
    auto it = blocks.begin();
    while (it != blocks.end()) {
        auto next = std::next(it);
        if (it->pid == -1 && next != blocks.end() && next->pid == -1) {
            // 当前块和下一块都是 FREE，合并
            it->size += next->size;
            blocks.erase(next);  // 删除下一块
            // 不推进 it，继续检查当前块与新的下一块是否也能合并
        } else {
            ++it;
        }
    }
}

int MemoryManager::bestFit(int pid, const std::string& name, int size) {
    auto best_fit = blocks.end();
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->pid == -1 && it->size >= size) {
            // 最佳适应：找 size 最小的够大的块（而不是地址最小的）
            if (best_fit == blocks.end() || it->size < best_fit->size) {
                best_fit = it;
            }
        }
    }

    if (best_fit == blocks.end()) { return -1; }

    int base = best_fit->start_address;

    // 若有剩余空间，切割出一块新的 FREE 块，插到当前块正后面
    int remaining = best_fit->size - size;
    if (remaining > 0) {
        MemBlock remaining_block;
        remaining_block.start_address = base + size;
        remaining_block.size = remaining;
        remaining_block.pid = -1;
        remaining_block.process_name = "FREE";
        // insert(pos, val)：插到 pos 之前，即当前块的紧邻后方
        blocks.insert(std::next(best_fit), remaining_block);
    }

    best_fit->pid = pid;
    best_fit->process_name = name;
    best_fit->size = size;

    return base;
}