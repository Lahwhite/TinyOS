#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    // count：信号量初始值
    explicit Semaphore(int count);

    void acquire();   // P 操作：等待（计数-1，若<0则阻塞）
    void release();   // V 操作：释放（计数+1，唤醒一个等待者）

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;
};