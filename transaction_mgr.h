/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:52:00
 * @edit: regangcli
 * @brief: 
 */
#pragma once

#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <climits>

#include "data_def.h"


// 交易管理器
class TransactionMgr
{
private:
    struct Compare {
        bool operator()(const Order* a, const Order* b) const {
            if (a.val == b.val) {
                return a.id < b.id;
            }
            return a.val < b.val;
        }
    };

public:
    TransactionMgr()
    : snapShorts_(1)
    {
        snapShorts_[0].lowestPrice = INT_MAX;
    };
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

private:
    std::unordered_map<int64_t, Order *> curOrders_; // 当前订单
    std::set<Order *, Compare> datas; 

    std::unordered_map<int64_t, std::vector<Transaction *>> pendingTransactions_;
    std::vector<RawSnapShort> snapShorts_;
    int tickTimeSpan_ = 0;
};
