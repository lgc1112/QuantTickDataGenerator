/*
 * @file: 
 * @Author: ligengchao
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:52:00
 * @edit: ligengchao
 * @brief: 
 */
#pragma once

#include <unordered_map>
#include <vector>
#include <set>
#include <queue>
#include <climits>

#include "data_def.h"

// 交易管理器
class TransactionMgr
{
public:
    TransactionMgr()
    : snapshots_(2)
    {
    }
    ~TransactionMgr() = default;

    // 将快照数据写入文件
    void DumpSnapshotToFile(const std::string &path);
    // 获取快照数量
    int GetSnapshotNum() { return snapshots_.size(); }
    // 获取快照
    const std::vector<RawSnapshot> &GetSnapshot() const { return snapshots_; }
    // 获取快照Map
    std::unordered_map<int64_t, std::vector<RawSnapshot *>> &GetSnapshotMap() { return snapshotsMap_; };
    // 设置preClosePrice
    void SetPreClosePrice(double price);

    // 接收order回调
    void OnReceiveOrder(void *data);
    // 接收transaction回调
    void OnReceiveTransaction(void *data);
    // tick回调
    void OnTick(int tickTimeSpan);

private:
    int _GetNextTickTimeSpan();

private:
    std::unordered_map<int64_t, Order *> curOrders_;  // 当前订单
    std::multiset<Order *, OrderCompare> buyOrders_;  // 买入订单有序集合(从小到大)
    std::multiset<Order *, OrderCompare> sellOrders_; // 卖出订单有序集合(从小到大)

    std::unordered_map<int64_t, std::vector<Transaction *>> pendingTransactions_;

    std::vector<RawSnapshot> snapshots_;
    std::unordered_map<int64_t, std::vector<RawSnapshot *>> snapshotsMap_;

    int tickTimeSpan_ = 0;
    double preClosePrice_ = 0;
    double upperLimitPrice_ = 0;
    double lowerLimitPrice_ = 0;
};
