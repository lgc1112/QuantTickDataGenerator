/*
 * @file: 
 * @Author: ligengchao
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:51:43
 * @edit: ligengchao
 * @brief: 
 */
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <system_error>
#include <unordered_map>
#include <climits>
#include <cmath>

#include "logger.h"
#include "transaction_mgr.h"

void TransactionMgr::DumpSnapshotToFile(const std::string &filename)
{
    std::ofstream file(filename);

    if (!file)
    {
        std::error_code ec(errno, std::generic_category());
        LOG_ERROR("Error opening file %s, err:%s'", filename.c_str(), ec.message().c_str());
        return;
    }

    OnTick(99 * 3600 + 59 * 60 + 59);
    LOG_INFO("DumpSnapshotToFile:%s", filename.c_str());
    LOG_INFO("pendingTransactions_:%lu", pendingTransactions_.size());
    LOG_INFO("curOrders_:%lu", curOrders_.size());

    // 写入 CSV 文件头部
    file << "InstrumentID,TradingDay,UpdateTime,UpdateMillisec,RefUpdateTime,RefUpdateMillisec,LastPrice,Volume,"
            "LastVolume,Turnover,LastTurnover,"
         << "AskPrice5,AskPrice4,AskPrice3,AskPrice2,AskPrice1,BidPrice1,BidPrice2,BidPrice3,BidPrice4,BidPrice5,"
         << "AskVolume5,AskVolume4,AskVolume3,AskVolume2,AskVolume1,BidVolume1,BidVolume2,BidVolume3,BidVolume4,"
            "BidVolume5,"
         << "OpenInterest,UpperLimitPrice,LowerLimitPrice,HighestPrice,LowestPrice,PreClosePrice\n";

    // 遍历数组并写入每个快照
    for (const auto &snapshot : snapshots_)
    {
        file << "SZ127080"
             << "," << 20230619 << "," << snapshot.updateTime << "," << std::setw(3) << std::setfill('0')
             << snapshot.updateMillisec << ","
             << "0"
             << "," << std::setw(3) << std::setfill('0') << snapshot.refUpdateMicrosec << "," << std::fixed
             << std::setprecision(8) << snapshot.lastPrice << "," << snapshot.volume << "," << snapshot.lastVolume
             << "," << (double)(int)snapshot.turnover << "," << (double)(int)snapshot.lastTurnover << ","
             << snapshot.askPrice5 << "," << snapshot.askPrice4 << "," << snapshot.askPrice3 << ","
             << snapshot.askPrice2 << "," << snapshot.askPrice1 << "," << snapshot.bidPrice1 << ","
             << snapshot.bidPrice2 << "," << snapshot.bidPrice3 << "," << snapshot.bidPrice4 << ","
             << snapshot.bidPrice5 << "," << snapshot.askVolume5 << "," << snapshot.askVolume4 << ","
             << snapshot.askVolume3 << "," << snapshot.askVolume2 << "," << snapshot.askVolume1 << ","
             << snapshot.bidVolume1 << "," << snapshot.bidVolume2 << "," << snapshot.bidVolume3 << ","
             << snapshot.bidVolume4 << "," << snapshot.bidVolume5 << "," << 0 << "," << upperLimitPrice_ << ","
             << lowerLimitPrice_ << "," << snapshot.highestPrice << "," << snapshot.lowestPrice << "," << preClosePrice_
             << "\n";
    }

    file.close();
}

void TransactionMgr::SetPreClosePrice(double price)
{
    preClosePrice_ = price;
    upperLimitPrice_ = std::round(preClosePrice_ * 1.2 * 1000) / 1000.0;
    lowerLimitPrice_ = std::round(preClosePrice_ * 0.8 * 1000) / 1000.0;
}

void TransactionMgr::OnReceiveOrder(void *data)
{
    auto order = static_cast<Order *>(data);

    int secondsInDay = (order->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
    OnTick(secondsInDay);
    // // 超时，需要tick
    // int secondsInDay = (order->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
    // // secondsInDay = 14 * 3600 + 57 * 60;
    // int nextTick = _GetNextTickTimeSpan();
    // while (secondsInDay >= nextTick)
    // {
    //     static int count = 0;
    //     if (++count >= 10)
    //         return;

    //     LOG_INFO("tickTimeSpan_: %d(%02d:%02d:%02d), transaction:[%s], pendingTransactions: %lu",
    //              tickTimeSpan_,
    //              tickTimeSpan_ / 3600,
    //              tickTimeSpan_ / 60 % 60,
    //              tickTimeSpan_ % 60,
    //              order->ToString().c_str(),
    //              pendingTransactions_.size());
    //     tickTimeSpan_ = nextTick;
    //     OnTick(tickTimeSpan_);
    //     nextTick = _GetNextTickTimeSpan();
    // }

    auto orderID = order->orderID;
    if (curOrders_.find(orderID) != curOrders_.end())
    {
        LOG_ERROR1("order id error");
        return;
    }

    if (order->isBuy)
        order->buyOrdersIter = buyOrders_.emplace(order);
    else
        order->sellOrdersIter = sellOrders_.emplace(order);

    curOrders_.emplace(orderID, order);

    // 处理正在等待order的transaction
    auto it = pendingTransactions_.find(orderID);
    if (it != pendingTransactions_.end())
    {
        auto &transactions = it->second;
        // LOG_DEBUG("erase pendding orderId:%lld, transactions:%lu", orderID, transactions.size());
        // 遍历处理所有等待的transaction
        for (auto transaction : transactions)
        {
            // LOG_DEBUG("erase pendding orderId:%lld, pendingTransactions_:%lu", orderID, pendingTransactions_.size());
            OnReceiveTransaction(transaction);
        }

        pendingTransactions_.erase(it);
    }

    // std::cout << count << ": " << order->ToString() << std::endl;
    // if (++count == 10)
    //     exit(0);
}

void TransactionMgr::OnReceiveTransaction(void *data)
{
    auto transaction = static_cast<Transaction *>(data);

    // 超时，需要tick
    int secondsInDay = (transaction->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
    // secondsInDay = 14 * 3600 + 57 * 60;
    int nextTick = _GetNextTickTimeSpan();
    if (secondsInDay >= nextTick)
    {
        while (secondsInDay >= nextTick)
        {
            // static int count = 0;
            // if (++count >= 10)
            //     return;

            // LOG_INFO("tickTimeSpan_: %d(%02d:%02d:%02d), transaction:[%s], pendingTransactions: %lu",
            //          tickTimeSpan_,
            //          tickTimeSpan_ / 3600,
            //          tickTimeSpan_ / 60 % 60,
            //          tickTimeSpan_ % 60,
            //          transaction->ToString().c_str(),
            //          pendingTransactions_.size());
            tickTimeSpan_ = nextTick;
            OnTick(tickTimeSpan_);
            nextTick = _GetNextTickTimeSpan();
        }
    }
    else
    {
        OnTick(secondsInDay);
    }
    // LOG_INFO("tickTimeSpan_: %d, nextTick: %d", secondsInDay, nextTick);
    // exit(0);

    auto askOrderID = transaction->askOrderID;
    auto bidOrderID = transaction->bidOrderID;

    // // 校验数据是否有问题
    // if (askOrderID == 0 && bidOrderID == 0)
    // {
    //     LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
    //     return;
    // }

    // if (askOrderID == bidOrderID)
    // {
    //     LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
    //     return;
    // }

    // if (transaction->isCancel && askOrderID != 0 && bidOrderID != 0)
    // {
    //     LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
    //     return;
    // }

    // if (!transaction->isCancel && (askOrderID == 0 || bidOrderID == 0))
    // {
    //     LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
    //     return;
    // }

    auto askIt = askOrderID == 0 ? curOrders_.end() : curOrders_.find(askOrderID);
    auto bidIt = bidOrderID == 0 ? curOrders_.end() : curOrders_.find(bidOrderID);

    // 如果不是成交，则只要有一个订单不满足就挂起不处理
    if (!transaction->isCancel)
    {
        // 卖单不存在,则挂起等待卖单
        if (askIt == curOrders_.end())
        {
            // LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            pendingTransactions_[askOrderID].push_back(transaction);
            return;
        }

        // 买单不存在,则挂起等待买单
        if (bidIt == curOrders_.end())
        {
            // LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            pendingTransactions_[bidOrderID].push_back(transaction);
            return;
        }
    }
    // 撤单
    else
    {
        if (askOrderID != 0 && (askIt == curOrders_.end()))
        {
            // LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            pendingTransactions_[askOrderID].push_back(transaction);
            return;
        }

        if (bidOrderID != 0 && (bidIt == curOrders_.end()))
        {
            // LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            pendingTransactions_[bidOrderID].push_back(transaction);
            return;
        }
    }

    // 校验完成，结单
    if (askIt != curOrders_.end())
    {
        auto remainVolume = askIt->second->orderVolume - transaction->orderVolume;
        if (remainVolume < 0)
        {
            LOG_ERROR("err remainVolume, askOrderID: %lld, bidOrderID: %lld, remainVolume: %d, transaction:[%s], "
                      "order:[%s]",
                      askOrderID,
                      bidOrderID,
                      remainVolume,
                      transaction->ToString().c_str(),
                      curOrders_.find(askOrderID)->second->ToString().c_str());
            return;
        }

        // 该订单已全部取消
        if (remainVolume == 0)
        {
            if (askIt->second->isBuy)
                buyOrders_.erase(askIt->second->buyOrdersIter);
            else
                sellOrders_.erase(askIt->second->sellOrdersIter);

            curOrders_.erase(askIt);
            bidIt = bidOrderID == 0 ? curOrders_.end() : curOrders_.find(bidOrderID);
            // LOG_ERROR("err curOrders_ %lu", curOrders_.size());
        }
        // 修改订单数量
        else
        {
            askIt->second->orderVolume = remainVolume;
        }
    }

    if (bidIt != curOrders_.end())
    {
        auto remainVolume = bidIt->second->orderVolume - transaction->orderVolume;
        if (remainVolume < 0)
        {
            LOG_ERROR("err remainVolume, askOrderID: %lld, bidOrderID: %lld, remainVolume: %d",
                      askOrderID,
                      bidOrderID,
                      remainVolume);
            return;
        }

        // 该订单已全部取消
        if (remainVolume == 0)
        {
            if (bidIt->second->isBuy)
                buyOrders_.erase(bidIt->second->buyOrdersIter);
            else
                sellOrders_.erase(bidIt->second->sellOrdersIter);

            curOrders_.erase(bidIt);
            // LOG_ERROR("err curOrders_ %lu", curOrders_.size());
        }
        // 修改订单数量
        else
        {
            bidIt->second->orderVolume = remainVolume;
        }
    }

    // 如果是成交，记录交易快照
    if (!transaction->isCancel)
    {
        auto transVolume = transaction->orderVolume;
        auto lastPrice = transaction->orderPrice;
        auto turnover = transaction->orderPrice * transVolume;

        auto &snapshot = snapshots_.back();
        snapshot.lastPrice = lastPrice;
        snapshot.volume += transVolume;
        snapshot.lastVolume += transVolume;
        snapshot.turnover += turnover;
        snapshot.lastTurnover += turnover;
        snapshot.highestPrice = std::max(snapshot.highestPrice, lastPrice);
        snapshot.lowestPrice = std::min(snapshot.lowestPrice, lastPrice);
    }

    // std::cout << count << "Recv Transaction : " << transaction->ToString() << std::endl;
    // if (++count == 1000)
    //     exit(0);
}

void TransactionMgr::OnTick(int tickTimeSpan)
{
    // 更新快照
    auto &snapshot = snapshots_.back();
    sprintf(snapshot.updateTime, "%02d:%02d:%02d", tickTimeSpan / 3600, tickTimeSpan / 60 % 60, tickTimeSpan % 60);
    // LOG_INFO("tickTimeSpan: %d, %d, %s", tickTimeSpan, tickTimeSpan % 60, snapshot.updateTime);

    // if (curOrders_.size() != buyOrders_.size() + sellOrders_.size())
    //     LOG_ERROR("curOrders_ size: %lu, buyOrders_ size + sellOrders_ size: %lu",
    //               curOrders_.size(),
    //               buyOrders_.size() + sellOrders_.size());

    // LOG_DEBUG("max buyOrders_: %.3f, min sellOrders_: %.3f", (*buyOrders_.rbegin())->orderPrice, (*sellOrders_.begin())->orderPrice);
    std::array<std::pair<double, int64_t>, 5> max5BuyPrices_;  // {prices : volume}
    std::array<std::pair<double, int64_t>, 5> min5SellPrices_; // {prices : volume}
    int idx = 0;
    for (auto it = buyOrders_.rbegin(); it != buyOrders_.rend(); it++)
    {
        // 排除超前订单
        int secondsInDay = ((*it)->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
        if (secondsInDay > tickTimeSpan)
            continue;

        auto prices = (*it)->orderPrice;
        auto maxPrice = max5BuyPrices_[idx].first;
        if (prices == maxPrice)
        {
            max5BuyPrices_[idx].second += (*it)->orderVolume;
        }
        else
        {
            if (idx == 4 && maxPrice != 0)
                break;

            max5BuyPrices_[maxPrice == 0 ? idx : ++idx] = {prices, (*it)->orderVolume};
        }
    }

    idx = 0;
    for (auto it = sellOrders_.begin(); it != sellOrders_.end(); it++)
    {
        // 排除超前订单
        int secondsInDay = ((*it)->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
        if (secondsInDay > tickTimeSpan)
            continue;

        auto prices = (*it)->orderPrice;
        auto minPrice = min5SellPrices_[idx].first;
        if (prices == minPrice)
        {
            min5SellPrices_[idx].second += (*it)->orderVolume;
        }
        else
        {
            if (idx == 4 && minPrice != 0)
                break;

            min5SellPrices_[minPrice == 0 ? idx : ++idx] = {prices, (*it)->orderVolume};
        }
    }

    // 保存到快照
    snapshot.askPrice1 = min5SellPrices_[0].first;
    snapshot.askVolume1 = min5SellPrices_[0].second;
    snapshot.askPrice2 = min5SellPrices_[1].first;
    snapshot.askVolume2 = min5SellPrices_[1].second;
    snapshot.askPrice3 = min5SellPrices_[2].first;
    snapshot.askVolume3 = min5SellPrices_[2].second;
    snapshot.askPrice4 = min5SellPrices_[3].first;
    snapshot.askVolume4 = min5SellPrices_[3].second;
    snapshot.askPrice5 = min5SellPrices_[4].first;
    snapshot.askVolume5 = min5SellPrices_[4].second;

    snapshot.bidPrice1 = max5BuyPrices_[0].first;
    snapshot.bidVolume1 = max5BuyPrices_[0].second;
    snapshot.bidPrice2 = max5BuyPrices_[1].first;
    snapshot.bidVolume2 = max5BuyPrices_[1].second;
    snapshot.bidPrice3 = max5BuyPrices_[2].first;
    snapshot.bidVolume3 = max5BuyPrices_[2].second;
    snapshot.bidPrice4 = max5BuyPrices_[3].first;
    snapshot.bidVolume4 = max5BuyPrices_[3].second;
    snapshot.bidPrice5 = max5BuyPrices_[4].first;
    snapshot.bidVolume5 = max5BuyPrices_[4].second;

    int size = snapshots_.size();
    // 去重
    if (snapshots_[size - 2].Compare(snapshot))
    {
        // LOG_INFO("snapshots_[size - 2] == snapshots_[size - 1], size: %lu", size);
        return;
    }

    // 保存快照信息
    auto highestPrice = snapshot.highestPrice;
    auto lowestPrice = snapshot.lowestPrice;
    auto lastPrice = snapshot.lastPrice;
    auto volume = snapshot.volume;
    auto turnover = snapshot.turnover;

    // // 保存到map
    // snapshotsMap_[volume].emplace_back(&snapshot);

    // 切换到下一个快照
    snapshots_.emplace_back();
    auto &lastSnapShort = snapshots_.back();
    lastSnapShort.highestPrice = highestPrice;
    lastSnapShort.lowestPrice = lowestPrice;
    lastSnapShort.lastPrice = lastPrice;
    lastSnapShort.volume = volume;
    // lastSnapShort.lastVolume = 0;
    lastSnapShort.turnover = turnover;
    // lastSnapShort.lastTurnover = 0;
}

// 打点时间
// 9:25:00
// 9:25:03
// 9:26:30
// 9:27:30
// 9:28:30
// 9:29:30
// 9:30:03

// （9:30:00 -> 11:30:] 每3s一个tick

// 11:30:03
// 11:40:30
// 11:51:30
// 12:00:30
// 12:11:30
// 12:22:30
// 12:33:30
// 12:44:30
// 12:55:30  13:00:03

// (13:00:00 14:57:00】 每3s一个tick
int TransactionMgr::_GetNextTickTimeSpan()
{
    // 特殊tick映射
    static std::unordered_map<int, int> nextTickSpanMap = {
        // 9:25:00
        // 9:25:03
        // 9:26:30
        // 9:27:30
        // 9:28:30
        // 9:29:30
        // 9:30:03
        {9 * 3600 + 25 * 60 + 0, 9 * 3600 + 25 * 60 + 3},
        {9 * 3600 + 25 * 60 + 3, 9 * 3600 + 26 * 60 + 30},
        {9 * 3600 + 26 * 60 + 30, 9 * 3600 + 27 * 60 + 30},
        {9 * 3600 + 27 * 60 + 30, 9 * 3600 + 28 * 60 + 30},
        {9 * 3600 + 28 * 60 + 30, 9 * 3600 + 29 * 60 + 30},
        {9 * 3600 + 29 * 60 + 30, 9 * 3600 + 30 * 60 + 3},

        // 11:30:00
        // 11:30:03
        // 11:40:30
        // 11:51:30
        // 12:00:30
        // 12:11:30
        // 12:22:30
        // 12:33:30
        // 12:44:30
        // 12:55:30  13:00:03
        {11 * 3600 + 30 * 60 + 0, 11 * 3600 + 30 * 60 + 3},
        {11 * 3600 + 30 * 60 + 3, 11 * 3600 + 40 * 60 + 30},
        {11 * 3600 + 40 * 60 + 30, 11 * 3600 + 51 * 60 + 30},
        {11 * 3600 + 51 * 60 + 30, 12 * 3600 + 0 * 60 + 30},
        {12 * 3600 + 0 * 60 + 30, 12 * 3600 + 11 * 60 + 30},
        {12 * 3600 + 11 * 60 + 30, 12 * 3600 + 22 * 60 + 30},
        {12 * 3600 + 22 * 60 + 30, 12 * 3600 + 33 * 60 + 30},
        {12 * 3600 + 33 * 60 + 30, 12 * 3600 + 44 * 60 + 30},
        {12 * 3600 + 44 * 60 + 30, 12 * 3600 + 55 * 60 + 30},
        {12 * 3600 + 55 * 60 + 30, 13 * 3600 + 00 * 60 + 3},

    };

    // （9:30:00 -> 11:30:] (13:00:00 14:57:00] 每3s一个tick
    if ((tickTimeSpan_ > 9 * 3600 + 30 * 60 && tickTimeSpan_ < 11 * 3600 + 30 * 60) ||
        (tickTimeSpan_ > 13 * 3600 && tickTimeSpan_ < 14 * 3600 + 57 * 60))
    {
        return tickTimeSpan_ + 3;
    }

    // （9:25:00) 才第一个tick
    if (tickTimeSpan_ < 9 * 3600 + 25 * 60)
    {
        return 9 * 3600 + 25 * 60;
    }

    if (nextTickSpanMap.find(tickTimeSpan_) != nextTickSpanMap.end())
    {
        return nextTickSpanMap[tickTimeSpan_];
    }

    return INT_MAX;
}