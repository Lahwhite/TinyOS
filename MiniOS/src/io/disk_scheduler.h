#pragma once

#include <vector>

class DiskScheduler {
public:
    // 初始参数：磁道总数、磁头初始位置
    DiskScheduler(int num_tracks, int initial_head);

    // 添加一个磁盘请求（磁道号）
    void addRequest(int track);

    // 运行指定算法，返回总移动距离
    int run(const std::string& algorithm, int direction = 1);
    // direction: 1=向高磁道, -1=向低磁道（SCAN 初始方向）

    // 打印磁头移动轨迹
    void printTrajectory() const;

private:
    int num_tracks;
    int initial_head;
    std::vector<int> requests;      // 原始请求列表（按到来顺序）
    std::vector<int> service_order; // 运行后的服务顺序
};