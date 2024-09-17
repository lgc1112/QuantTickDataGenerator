/*
 * @file: 
 * @Author: ligengchao
 * @copyright: 
 * @Date: 2024-09-16 22:50:37
 * @edit: ligengchao
 * @brief: 
 */
#pragma once

#include <array>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>

#include "data_def.h"

// 实盘数据事件类型
enum DataEventType
{
    RECV_ORDER = 0,
    RECV_TRANSACTION,
    END,
};

// 实盘数据模拟器
class LiveTradingDataSimulator
{
#pragma region : ctor /dtor
public:
    LiveTradingDataSimulator() = default;
    ~LiveTradingDataSimulator() = default;
#pragma endregion

#pragma region : 对外 api
public:
    // 初始化函数
    int Init();

    // 运行
    void Run();

    // 注册事件处理器
    void RegisterEventHandler(DataEventType event, std::function<void(void *)> handler);
#pragma endregion

#pragma region : 私有方法
private:
    // 从CSV文件中读取Order数据
    void _ReadRawOrders(const std::string &filename, std::vector<Order> &orders);
    // 从CSV文件中读取Transaction数据
    void _ReadRawTransactions(const std::string &filename, std::vector<Transaction> &transactions);
#pragma endregion

#pragma region : 私有成员
private:
    std::array<std::vector<std::function<void(void *)>>, DataEventType::END> eventHandlers_;

    std::vector<Order> orders_;
    std::vector<Transaction> transactions_;
#pragma endregion
};

