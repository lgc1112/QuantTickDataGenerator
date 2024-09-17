/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 22:50:37
 * @edit: regangcli
 * @brief: 
 */
#pragma once

#include "data_def.h"

// 数据事件类型
enum DataEventType
{
    RECV_ORDER = 0,
    RECV_TRANSACTION,
    END,
};

// 实盘数据模拟器
class LiveTradingDataSimulator
{
public:
    LiveTradingDataSimulator() = default;
    ~LiveTradingDataSimulator() = default;

    // 初始化函数
    int Init();

    // 初始化函数
    void Run();

    // 注册事件处理器
    void RegisterEventHandler(DataEventType event, std::function<void(void *)> handler);

    // 检查Tick存在百分比
    double CheckTickDataExistPercent(const std::string &filename);

private:
    void _ReadRawOrders(const std::string &filename, std::vector<Order> &orders);
    void _ReadRawTransactions(const std::string &filename, std::vector<Transaction> &transactions);
    void _ReadRawSnapshots(const std::string &filename, std::vector<RawSnapshot> &snapshots);
    // 事件处理器列表
    std::array<std::vector<std::function<void(void *)>>, DataEventType::END> eventHandlers_;

    std::vector<Order> orders_;
    std::vector<Transaction> transactions_;
};

