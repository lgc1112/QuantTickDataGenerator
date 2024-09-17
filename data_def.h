/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 08:53:49
 * @edit: regangcli
 * @brief: 
 */
#pragma once

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <cstring> // 包含对 memcpy 的声明
#include <string>
#include <set>
#include <climits>

struct RawOrder
{
    char instrumentId[32];     // 合约代码
    char tradingDay[9];        // 委托日期
    char updateTime[9];        // 交易所发布时间（时分秒）
    int updateMillisec = 0;    // 交易所发布时间（毫秒字段）
    char refUpdateTime[9];     // 本地接收到消息的时间（时分秒）
    int refUpdateMicrosec = 0; // 本地接收到消息的时间（微秒字段）
    uint64_t orderSysID = 0;   // 委托号，交易所Order唯一编号
    double orderPrice = 0;     // 委托价格
    int64_t orderVolume = 0;   // 委托数量
    char direction;            // 买卖方向（1=买，2=卖，G=借入，F=出借）
    char orderType;            // 订单类型（1=市价，2=限价，U=本方最优）
    uint32_t channelId = 0;    // 通道号（本题目可以不用管）

    std::string ToString() const;
};

struct RawTransaction
{
    char instrumentId[32];     // 合约代码
    char tradingDay[9];        // 成交日期
    char updateTime[9];        // 交易所发布时间（时分秒）
    int updateMillisec = 0;    // 交易所发布时间（毫秒字段）
    char refUpdateTime[9];     // 本地接收到消息的时间（时分秒）
    int refUpdateMicrosec = 0; // 本地接收到消息的时间（微秒字段）
    uint64_t tradeId = 0;      // 成交编号
    double tradePrice = 0;     // 成交价格
    int64_t tradeVolume = 0;   // 成交数量（股票：股；权证：份；债券：张）
    double turnover = 0;       // 成交金额
    char direction = 0;        // 买卖方向（'0'-无，'1'=买，'2'=卖）
    char orderKind = 0;        // 成交类别（'0'-无）
    char functionCode = 0;     // 成交代码（'0'=成交，'C'=撤单）
    uint64_t askOrderID = 0;   // 卖方报单号
    uint64_t bidOrderID = 0;   // 买方报单号
    uint32_t channelId = 0;    // 通道号（本题目可以不用管）

    std::string ToString() const;
};

struct RawSnapShort
{
    char instrumentId[32]; // 合约代码
    char tradingDay[9];    // 成交日期

    char updateTime[9];     // 交易所发布时间（时分秒）
    int updateMillisec = 0; // 交易所发布时间（毫秒字段）

    char refUpdateTime[9]; // 本地接收到消息的时间（时分秒）
    int refUpdateMicrosec; // 本地接收到消息的时间（微秒字段）

    double lastPrice = 0;    // 最新成交价
    int64_t volume = 0;      // 开盘到本发布时间累计成交量
    int64_t lastVolume = 0;  // 上次发布时间到本发布时间累计成交量
    double turnover = 0;     //  开盘到本发布时间累计成交金额
    double lastTurnover = 0; // 上次发布时间到本发布时间累计成交金额

    double askPrice5 = 0, askPrice4 = 0, askPrice3 = 0, askPrice2 = 0, askPrice1 = 0; // 卖5，卖4，卖3，卖2，卖1盘口价格
    double bidPrice1 = 0, bidPrice2 = 0, bidPrice3 = 0, bidPrice4 = 0, bidPrice5 = 0; // 买1，买2，买3，买4，买5盘口价格

    int64_t askVolume5 = 0, askVolume4 = 0, askVolume3 = 0, askVolume2 = 0,
            askVolume1 = 0; // 卖5，卖4，卖3，卖2，卖1盘口挂单量
    int64_t bidVolume1 = 0, bidVolume2 = 0, bidVolume3 = 0, bidVolume4 = 0,
            bidVolume5 = 0; // 买1，买2，买3，买4，买5盘口挂单量

    int64_t openInterest = 0; // 忽略，不要管，无需关注

    double upperLimitPrice = 0; // 涨停板价
    double lowerLimitPrice = 0; // 跌停板价
    double highestPrice = 0;    // 最高价
    double lowestPrice = INT_MAX;     // 最低价
    double preClosePrice = 0;   // 昨天收盘价

    std::string ToString() const;
    bool Compare(const RawSnapShort &other) const;
};

struct Order;
class OrderCompare
{
public:
    bool operator()(const Order *a, const Order *b) const;
};

struct Order
{
    uint64_t orderID = 0;          // 委托号，交易所Order唯一编号
    int64_t updateTimeSpan = 0;    // 本地接收到消息的时间，微秒级时间戳
    int64_t refUpdateTimeSpan = 0; // 本地接收到消息的时间，微秒级时间戳
    double orderPrice = 0;         // 委托价格
    int64_t orderVolume = 0;       // 委托数量
    bool isBuy = false;            // 买卖方向（1=买，2=卖）

    std::set<Order *, OrderCompare>::iterator buyOrdersIter;  // 买入订单有序集合迭代器
    std::set<Order *, OrderCompare>::iterator sellOrdersIter; // 卖出订单有序集合迭代器

    void FromRawOrder(const RawOrder &rawOrder);
    std::string ToString() const;
};

struct Transaction
{
    char instrumentId[32];         // 合约代码
    int64_t updateTimeSpan = 0;    // 本地接收到消息的时间，微秒级时间戳
    int64_t refUpdateTimeSpan = 0; // 本地接收到消息的时间，微秒级时间戳
    double orderPrice = 0;         // 委托价格
    int64_t orderVolume = 0;       // 委托数量
    bool isBuy = false;            // 买卖方向（1=买，2=卖）
    uint64_t askOrderID = 0;       // 卖方报单号
    uint64_t bidOrderID = 0;       // 买方报单号
    bool isCancel = false;         // 是否为撤单

    void FromRawTransaction(const RawTransaction &rawTransaction);

    std::string ToString() const;
};