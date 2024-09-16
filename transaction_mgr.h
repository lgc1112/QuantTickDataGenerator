/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:52:00
 * @edit: regangcli
 * @brief: 
 */
#pragma once

#include "data_def.h"
#include <string>
#include <unordered_map>
#include <vector>

// 交易管理器
class TransactionMgr
{
public:
    TransactionMgr()
    : snapShorts_(1){};
    ~TransactionMgr(){};

    // 将快照数据写入文件
    void DumpSnapshotToFile(const std::string &path); 
    // 获取快照数量
    int GetSnapshotNum() { return snapShorts_.size(); }; 

    // 接收order回调
    void OnReceiveOrder(void *data); 
    // 接收transaction回调
    void OnReceiveTransaction(void *data); 
    // tick回调
    void OnTick(int tickTimeSpan); 

private:
    int _GetNextTickTimeSpan();
    
    Transaction curTransaction_;
    std::unordered_map<int64_t, Order *> curOrders_;
    std::unordered_map<int64_t, std::vector<Transaction *>> pendingTransactions_;
    std::vector<RawSnapShort> snapShorts_;
    int tickTimeSpan_ = 0;
};
