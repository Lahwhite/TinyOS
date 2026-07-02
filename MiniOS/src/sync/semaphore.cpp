#include "semaphore.h"
#include <mutex>

Semaphore::Semaphore(int count) {
    this->count = count;
}

void Semaphore::acquire() {
    std::unique_lock<std::mutex> lock(this->mtx);
    while (count <= 0) {   // 等待直到 count > 0（while 防止虚假唤醒）
        cv.wait(lock);
    }
    count--;
}

void Semaphore::release() {
    std::unique_lock<std::mutex> lock(mtx);
    count ++;
    cv.notify_one();
}