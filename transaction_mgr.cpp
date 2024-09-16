/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:51:43
 * @edit: regangcli
 * @brief: 
 */
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <system_error>
#include <unordered_map>
#include <climits>

#include "transaction_mgr.h"
#include "logger.h"

void TransactionMgr::DumpSnapshotToFile(const std::string &filename)
{
    std::ofstream file(filename);

    if (!file)
    {
        std::error_code ec(errno, std::generic_category());
        LOG_ERROR("Error opening file %s, err:%s'", filename.c_str(), ec.message().c_str());
        return;
    }

    LOG_INFO("DumpSnapshotToFile:%s", filename.c_str());
    LOG_INFO("pendingCancelTransactions_:%lu", pendingCancelTransactions_.size());

    // 写入 CSV 文件头部
    file << "InstrumentID,TradingDay,UpdateTime,UpdateMillisec,RefUpdateTime,RefUpdateMillisec,LastPrice,Volume,"
            "LastVolume,Turnover,LastTurnover,"
         << "AskPrice5,AskPrice4,AskPrice3,AskPrice2,AskPrice1,BidPrice1,BidPrice2,BidPrice3,BidPrice4,BidPrice5,"
         << "AskVolume5,AskVolume4,AskVolume3,AskVolume2,AskVolume1,BidVolume1,BidVolume2,BidVolume3,BidVolume4,"
            "BidVolume5,"
         << "OpenInterest,UpperLimitPrice,LowerLimitPrice,HighestPrice,LowestPrice,PreClosePrice\n";

    // 遍历数组并写入每个快照
    for (const auto &snapshot : snapShorts_)
    {
        auto openInterest_ = 168.998;
        auto upperLimitPrice_ = 168.998 * 1.2;
        auto lowerLimitPrice_ = 168.998 * 0.8;
        file << "SZ127080"
             << "," << 20230619 << "," << snapshot.updateTime << "," << std::setw(3) << std::setfill('0')
             << snapshot.updateMillisec << ","
             << "0"
             << "," << std::setw(3) << std::setfill('0') << snapshot.refUpdateMicrosec << "," << snapshot.lastPrice
             << "," << snapshot.volume << "," << snapshot.lastVolume << "," << snapshot.turnover << ","
             << snapshot.lastTurnover << "," << snapshot.askPrice5 << "," << snapshot.askPrice4 << ","
             << snapshot.askPrice3 << "," << snapshot.askPrice2 << "," << snapshot.askPrice1 << ","
             << snapshot.bidPrice1 << "," << snapshot.bidPrice2 << "," << snapshot.bidPrice3 << ","
             << snapshot.bidPrice4 << "," << snapshot.bidPrice5 << "," << snapshot.askVolume5 << ","
             << snapshot.askVolume4 << "," << snapshot.askVolume3 << "," << snapshot.askVolume2 << ","
             << snapshot.askVolume1 << "," << snapshot.bidVolume1 << "," << snapshot.bidVolume2 << ","
             << snapshot.bidVolume3 << "," << snapshot.bidVolume4 << "," << snapshot.bidVolume5 << "," << openInterest_
             << "," << upperLimitPrice_ << "," << lowerLimitPrice_ << "," << snapshot.highestPrice << ","
             << snapshot.lowestPrice << "," << snapshot.preClosePrice << "\n";
    }

    file.close();
}

int count = 0;
void TransactionMgr::OnReceiveOrder(void *data)
{
    auto order = static_cast<Order *>(data);
    auto orderID = order->orderID;
    if (curOrders_.find(orderID) != curOrders_.end())
    {
        LOG_ERROR1("order id error");
        return;
    }

    curOrders_.emplace(orderID, order);
    // LOG_ERROR("err curOrders_ %lu", curOrders_.size()); 

    auto it = pendingCancelTransactions_.find(orderID);
    if (it != pendingCancelTransactions_.end())
    {
        LOG_DEBUG("erase pendding orderId:%lld", orderID);
        auto transaction = it->second;
        pendingCancelTransactions_.erase(it);
        auto peerOrderID = transaction->askOrderID == orderID ? transaction->bidOrderID : transaction->askOrderID;
        if (peerOrderID != 0)
        {
            auto peerIt = curOrders_.find(peerOrderID);
            if (peerIt != curOrders_.end())
            {
                LOG_DEBUG("erase pendding peerOrderID: %lld", peerOrderID);
                pendingCancelTransactions_.erase(peerOrderID);
            }
        }
        OnReceiveTransaction(transaction);
    }
    // LOG_DEBUG("order: %lld, refUpdateTimeSpan:%lld", order->orderID, order->refUpdateTimeSpan);

    // std::cout << count << ": " << order->ToString() << std::endl;
    // if (++count == 10)
    //     exit(0);
}

void TransactionMgr::OnReceiveTransaction(void *data)
{
    auto transaction = static_cast<Transaction *>(data);
    // if (transaction->isCancel)
    {
        auto askOrderID = transaction->askOrderID;
        auto bidOrderID = transaction->bidOrderID;

        // 校验数据是否有问题
        if (askOrderID == 0 && bidOrderID == 0)
        {
            LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            return;
        }

        if (askOrderID == bidOrderID)
        {
            LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            return;
        }

        if (transaction->isCancel && askOrderID != 0 && bidOrderID != 0)
        {
            LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            return;
        }

        if (!transaction->isCancel && (askOrderID == 0 || bidOrderID == 0))
        {
            LOG_ERROR("invalid order, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            return;
        }

        auto askIt = askOrderID == 0 ? curOrders_.end() : curOrders_.find(askOrderID);
        auto bidIt = bidOrderID == 0 ? curOrders_.end() : curOrders_.find(bidOrderID);

        // 如果不是成交，则只要有一个订单不满足就不处理
        if (!transaction->isCancel)
        {
            // 任意一个订单不存在
            if (askIt == curOrders_.end() || bidIt == curOrders_.end())
            {
                LOG_INFO("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
                pendingCancelTransactions_.emplace(askOrderID, transaction);
                pendingCancelTransactions_.emplace(bidOrderID, transaction);
                return;
            }

            // // 任意一个订单的订单量小于交易量
            // if (askIt->second->orderVolume < transaction->orderVolume ||
            //     bidIt->second->orderVolume < transaction->orderVolume)
            // {
            //     LOG_ERROR("Invalid trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
            //     // pendingCancelTransactions_.emplace(askOrderID, transaction);
            //     // pendingCancelTransactions_.emplace(bidOrderID, transaction);
            //     return;
            // }
        }
        // 撤单
        else
        {
            if (askOrderID != 0 && (askIt == curOrders_.end()))
            {
                LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
                pendingCancelTransactions_[askOrderID].push_back(transaction);
                return;
            }

            if (bidOrderID != 0 && (bidIt == curOrders_.end()))
            {
                LOG_WARN("pending trade, askOrderID: %lld, bidOrderID: %lld", askOrderID, bidOrderID);
                pendingCancelTransactions_[bidOrderID].push_back(transaction);
                // pendingCancelTransactions_.emplace(bidOrderID, transaction);
                return;
            }
        }

        // 校验完成，结单
        if (askIt != curOrders_.end())
        {
            auto remainVolume = askIt->second->orderVolume - transaction->orderVolume;
            if (remainVolume < 0)
            {
                // LOG_ERROR("err curOrders_ %lu", curOrders_.size()); 
                // for (auto &pair : curOrders_)
                // {
                //     if (pair.first != pair.second->orderID)
                //     {
                //         LOG_ERROR("err remainVolume, orderID: %lld,order:[%s]",
                //                   pair.first,
                //                   pair.second->ToString().c_str());
                //     }
                //     // LOG_ERROR("err remainVolume, askOrderID: %lld, bidOrderID: %lld, remainVolume: %d, order:[%s]",
                //     //           askOrderID,
                //     //           bidOrderID,
                //     //           remainVolume,
                //     //           iorder.second->ToString().c_str());
                // }

                LOG_ERROR("err remainVolume, askOrderID: %lld, bidOrderID: %lld, remainVolume: %d, transaction:[%s], "
                          "order:[%s]",
                          askOrderID,
                          bidOrderID,
                          remainVolume,
                          transaction->ToString().c_str(), curOrders_.find(askOrderID)->second->ToString().c_str());
                return;
            }

            // 该订单已全部取消
            if (remainVolume == 0)
            {
                curOrders_.erase(askIt);
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
                curOrders_.erase(bidIt);
            }
            // 修改订单数量
            else
            {
                bidIt->second->orderVolume = remainVolume;
            }
        }
    }

    // 超时，需要tick
    int secondsInDay = (transaction->updateTimeSpan / 1000000 + 8 * 3600) % 86400;
    // secondsInDay = 14 * 3600 + 57 * 60;
    int nextTick = _GetNextTickTimeSpan();
    while (secondsInDay >= nextTick)
    {
        LOG_INFO("tickTimeSpan_: %d(%d:%d:%d)",
                 tickTimeSpan_,
                 tickTimeSpan_ / 3600,
                 tickTimeSpan_ / 60 % 60,
                 tickTimeSpan_ % 60);
        OnTick(tickTimeSpan_);
        tickTimeSpan_ = nextTick;
        nextTick = _GetNextTickTimeSpan();
    }
    // LOG_INFO("tickTimeSpan_: %d, nextTick: %d", secondsInDay, nextTick);
    // exit(0);

    // 如果是成交，记录快照
    if (!transaction->isCancel)
    {
        auto transVolume = transaction->orderVolume;
        auto lastPrice = transaction->orderPrice;
        auto turnover = transaction->orderPrice * transVolume;

        auto &snapShort = snapShorts_.back();
        snapShort.lastPrice = lastPrice;
        snapShort.volume += transVolume;
        snapShort.lastVolume += transVolume;
        snapShort.turnover += turnover;
        snapShort.lastTurnover += turnover;
        snapShort.highestPrice = std::max(snapShort.highestPrice, lastPrice);
        snapShort.lowestPrice = std::min(snapShort.lowestPrice, lastPrice);
    }

    // std::cout << count << "Recv Transaction : " << transaction->ToString() << std::endl;
    if (++count == 1000)
        exit(0);
}

void TransactionMgr::OnTick(int tickTimeSpan)
{
    // 更新快照
    auto &snapShort = snapShorts_.back();
    sprintf(snapShort.updateTime, "%d:%d:%d", tickTimeSpan / 3600, tickTimeSpan / 60 % 60, tickTimeSpan_ % 60);
    // snapShort.updateTime = tickTimeSpan;

    std::vector<std::pair<double, int64_t>> max5BuyPrices_(5);  // {prices : volume}
    std::vector<std::pair<double, int64_t>> min5SellPrices_(5); // {prices : volume}
    // std::vector<std::pair<double, int64_t>> min5SellPrices_(5, {std::numeric_limits<double>::max(), 0}); // {prices : volume}

    // 遍历orders
    for (const auto &pair : curOrders_)
    {
        auto prices = pair.second->orderPrice;
        // 生成max5BuyPrices_集合
        if (max5BuyPrices_.back().first <= prices)
        {
            // 插入排序
            for (int i = 4; i >= 0; i--)
            {
                // 存在相等的价格, 更新交易量即可
                if (max5BuyPrices_[i].first == prices)
                {
                    max5BuyPrices_[i].second += pair.second->orderVolume;
                    break;
                }

                // 如果找到了队头或找到了前一个元素比当前价格更大，说明此处是插入位置
                if (i == 0 || max5BuyPrices_[i - 1].first > prices)
                {
                    max5BuyPrices_[4] = max5BuyPrices_[i];
                    max5BuyPrices_[i] = {prices, pair.second->orderVolume};
                    break;
                }

                // 否则继续往前找
            }
        }

        // // 生成min5SellPrices_集合
        // if (min5SellPrices_.back().first >= prices)
        // {
        //     // 插入排序
        //     for (int i = 4; i >= 0; i--)
        //     {
        //         // 存在相等的价格, 更新交易量即可
        //         if (min5SellPrices_[i].first == prices)
        //         {
        //             min5SellPrices_[i].second += pair.second->orderVolume;
        //             break;
        //         }

        //         // 如果找到了队头或找到了前一个元素比当前价格更小，说明此处是插入位置
        //         if (i == 0 ||  min5SellPrices_[i - 1].first < prices)
        //         {
        //             min5SellPrices_[4] = min5SellPrices_[i];
        //             min5SellPrices_[i] = {prices, pair.second->orderVolume};
        //             break;
        //         }

        //         // 否则继续往前找
        //     }
        // }

        // 生成min5SellPrices_集合
        auto backPrices = min5SellPrices_.back().first;
        if (backPrices >= prices || backPrices == 0)
        {
            // 插入排序
            for (int i = 4; i >= 0; i--)
            {
                // 存在相等的价格, 更新交易量即可
                if (min5SellPrices_[i].first == prices)
                {
                    min5SellPrices_[i].second += pair.second->orderVolume;
                    break;
                }

                // 如果找到了队头或找到了前一个元素比当前价格更小，说明此处是插入位置
                if (i == 0 || (min5SellPrices_[i - 1].first < prices && min5SellPrices_[i - 1].first != 0))
                {
                    min5SellPrices_[4] = min5SellPrices_[i];
                    min5SellPrices_[i] = {prices, pair.second->orderVolume};
                    break;
                }

                // 否则继续往前找
            }
        }
    }

    // 保存到快照
    snapShort.askPrice1 = max5BuyPrices_[0].first;
    snapShort.askVolume1 = max5BuyPrices_[0].second;
    snapShort.askPrice2 = max5BuyPrices_[1].first;
    snapShort.askVolume2 = max5BuyPrices_[1].second;
    snapShort.askPrice3 = max5BuyPrices_[2].first;
    snapShort.askVolume3 = max5BuyPrices_[2].second;
    snapShort.askPrice4 = max5BuyPrices_[3].first;
    snapShort.askVolume4 = max5BuyPrices_[3].second;
    snapShort.askPrice5 = max5BuyPrices_[4].first;
    snapShort.askVolume5 = max5BuyPrices_[4].second;

    // 保存到快照
    snapShort.bidPrice1 = min5SellPrices_[0].first;
    snapShort.bidVolume1 = min5SellPrices_[0].second;
    snapShort.bidPrice2 = min5SellPrices_[1].first;
    snapShort.bidVolume2 = min5SellPrices_[1].second;
    snapShort.bidPrice3 = min5SellPrices_[2].first;
    snapShort.bidVolume3 = min5SellPrices_[2].second;
    snapShort.bidPrice4 = min5SellPrices_[3].first;
    snapShort.bidVolume4 = min5SellPrices_[3].second;
    snapShort.bidPrice5 = min5SellPrices_[4].first;
    snapShort.bidVolume5 = min5SellPrices_[4].second;

    // 切换到下一个快照
    snapShorts_.push_back({});

    // 清理快照信息
    snapShorts_.back().lastVolume = 0;
    snapShorts_.back().lastPrice = 0;
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