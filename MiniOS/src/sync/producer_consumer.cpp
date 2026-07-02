#include <iostream>
#include <string>
#include <thread>

#include "producer_consumer.h"

void producerFunc(int id, int count,
                  std::queue<std::string>& buffer,
                  Semaphore& mutex, Semaphore& empty, Semaphore& full) {
    for (int i = 1; i <= count; i ++) {
        std::string s = std::string("id: ") + std::to_string(id) + " count: " + std::to_string(count);
        empty.acquire();
        mutex.acquire();
        buffer.push(s);
        std::cout << "[生产者" << id << "] 放入商品，缓冲区大小：" << buffer.size() << std::endl;
        mutex.release();
        full.release();
    }
}

void consumerFunc(int id, int count,
                  std::queue<std::string>& buffer,
                  Semaphore& mutex, Semaphore& empty, Semaphore& full) {
    for (int i = 1; i <= count; i ++) {
        std::string s = std::string("id: ") + std::to_string(id) + " count: " + std::to_string(count);
        full.acquire();
        mutex.acquire();
        buffer.pop();
        std::cout << "[消费者" << id << "] 取走商品，缓冲区大小：" << buffer.size() << std::endl;
        mutex.release();
        empty.release();
    }
}

void runProducerConsumer() {
    Semaphore mtx(1);
    Semaphore empty(5);
    Semaphore full(0);
    std::queue<std::string> buffer;

    std::thread p1(producerFunc, 1, 2, std::ref(buffer), std::ref(mtx), std::ref(empty), std::ref(full));
    std::thread p2(producerFunc, 2, 8, std::ref(buffer), std::ref(mtx), std::ref(empty), std::ref(full));
    std::thread p3(producerFunc, 3, 3, std::ref(buffer), std::ref(mtx), std::ref(empty), std::ref(full));

    std::thread c1(consumerFunc, 1, 10, std::ref(buffer), std::ref(mtx), std::ref(empty), std::ref(full));
    std::thread c2(consumerFunc, 2, 3, std::ref(buffer), std::ref(mtx), std::ref(empty), std::ref(full));

    p1.join(); p2.join(); p3.join();
    c1.join(); c2.join();
}