#pragma once

#include <list>
#include <string>

struct MemBlock {
    int start_address;
    int size;
    int pid;
    std::string process_name;
};

// 分配策略枚举
enum class AllocPolicy {
    FIRST_FIT,  // 首次适应：找第一个够大的块
    BEST_FIT,   // 最佳适应：找最小的够大的块
};

class MemoryManager {
public:
    // policy 默认首次适应，不传第二个参数时行为与之前完全一致
    MemoryManager(int total_kb, AllocPolicy policy = AllocPolicy::FIRST_FIT);

    // 打印当前内存布局（ASCII 可视化）
    void printLayout() const;

    // 统计并打印使用率
    void printStats() const;

    // 统一分配入口，内部按 policy 分派
    int allocate(int pid, const std::string& name, int size);

    void free(int pid);

    // 合并相邻的空闲块
    void mergeAdjacentFree();

private:
    int total_size;
    AllocPolicy policy;
    std::list<MemBlock> blocks;   // 内存块链表，按地址顺序排列

    // 两种具体算法，只在类内调用
    int firstFit(int pid, const std::string& name, int size);
    int bestFit (int pid, const std::string& name, int size);
};
