#pragma once

#include <queue>
#include <string>

#include "semaphore.h"

// 缓冲区大小
const int BUFFER_SIZE = 5;

// 生产者线程函数：生产 count 个商品
void producerFunc(int id, int count,
                  std::queue<std::string>& buffer,
                  Semaphore& mutex, Semaphore& empty, Semaphore& full);

// 消费者线程函数：消费 count 个商品
void consumerFunc(int id, int count,
                  std::queue<std::string>& buffer,
                  Semaphore& mutex, Semaphore& empty, Semaphore& full);

// 运行演示：3个生产者，2个消费者
void runProducerConsumer();