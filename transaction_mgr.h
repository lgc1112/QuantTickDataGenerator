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

// class OrderMgr
// {
// public:
//     OrderMgr();
//     ~OrderMgr();

//     int Init(const std::string &path);
//     Order *GetLatestOrder();
//     void UpdateLatestOrder();
//     void AddOrder(Order *order);
//     std::vector<std::pair<double, int64_t>> GetMax5BuyOders();  // [{委托价格, 委托数量}]
//     std::vector<std::pair<double, int64_t>> GetMin5SellOders(); // [{委托价格, 委托数量}]

// private:
//     std::ifstream file_;
//     Order curOrder_;
//     std::unordered_map<int64_t, Order> curOrders_;
// };


class TransactionMgr
{
public:
    TransactionMgr()
    : snapShorts_(1){};
    ~TransactionMgr() {};

    int Init(const std::string &path);
    void DumpSnapshotToFile(const std::string &path);

    void OnReceiveOrder(void *data);
    void OnReceiveTransaction(void *data);
    void OnTick();

private:
    Transaction curTransaction_;
    // std::unordered_map<int64_t, Transaction> curTransactions_;
    std::unordered_map<int64_t, Order *> curOrders_;
    std::unordered_map<int64_t, Transaction *> pendingCancelTransactions_;
    std::vector<RawSnapShort> snapShorts_;
    
};

