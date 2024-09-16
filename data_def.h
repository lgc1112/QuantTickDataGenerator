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

    std::string ToString() const
    {
        return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay +
               ", updateTime:" + updateTime + ", updateMillisec:" + std::to_string(updateMillisec) +
               ", refUpdateTime:" + refUpdateTime + ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) +
               ", orderSysID:" + std::to_string(orderSysID) + ", orderPrice:" + std::to_string(orderPrice) +
               ", orderVolume:" + std::to_string(orderVolume) + ", direction:" + direction +
               ", orderType:" + orderType + ", channelId:" + std::to_string(channelId);
    }
};

struct Order
{
    uint64_t orderID = 0;          // 委托号，交易所Order唯一编号
    int64_t updateTimeSpan = 0;    // 本地接收到消息的时间，微秒级时间戳
    int64_t refUpdateTimeSpan = 0; // 本地接收到消息的时间，微秒级时间戳
    double orderPrice = 0;         // 委托价格
    int64_t orderVolume = 0;       // 委托数量
    bool isBuy = false;            // 买卖方向（1=买，2=卖）

    void FromRawOrder(const RawOrder &rawOrder);

    std::string ToString() const
    {
        auto updateTimeMillisec = updateTimeSpan % 1000000 / 1000;
        char updateTime[9];
        sprintf(updateTime,
                "%d:%d:%d",
                (updateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
                updateTimeSpan / 1000000 / 60 % 60,
                updateTimeSpan / 1000000 % 60);

        auto refUpdateTimeMicrosec = refUpdateTimeSpan % 1000000;
        char refUpdateTime[9];
        sprintf(refUpdateTime,
                "%d:%d:%d",
                (refUpdateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
                refUpdateTimeSpan / 1000000 / 60 % 60,
                refUpdateTimeSpan / 1000000 % 60);

        return std::string("orderID:") + std::to_string(orderID) +
               ", updateTimeSpan:" + std::to_string(updateTimeSpan) + "(" + updateTime + "  " +
               std::to_string(updateTimeMillisec) + ")" + ", refUpdateTimeSpan:" + std::to_string(refUpdateTimeSpan) +
               "(" + refUpdateTime + " " + std::to_string(refUpdateTimeMicrosec) + ")" +
               ", orderPrice:" + std::to_string(orderPrice) + ", orderVolume:" + std::to_string(orderVolume) +
               ", isBuy:" + std::to_string(isBuy);
    }
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

    std::string ToString() const
    {
        return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay +
               ", updateTime:" + updateTime + ", updateMillisec:" + std::to_string(updateMillisec) +
               ", refUpdateTime:" + refUpdateTime + ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) +
               ", tradeId:" + std::to_string(tradeId) + ", tradePrice:" + std::to_string(tradePrice) +
               ", tradeVolume:" + std::to_string(tradeVolume) + ", turnover:" + std::to_string(turnover) +
               ", direction:" + direction + ", orderKind:" + orderKind + ", functionCode:" + functionCode +
               ", askOrderID:" + std::to_string(askOrderID) + ", bidOrderID:" + std::to_string(bidOrderID) +
               ", channelId:" + std::to_string(channelId);

        // return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay +
        //        ", updateTime:" + updateTime + ", updateMillisec:" + std::to_string(updateMillisec) +
        //        ", refUpdateTime:" + refUpdateTime + ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) +
        //        ", tradeId:" + std::to_string(tradeId) + ", tradePrice:" + std::to_string(tradePrice) +
        //        ", tradeVolume:" + std::to_string(tradeVolume) + ", turnover:" + std::to_string(turnover) +
        //        ", direction:" + std::to_string(direction) + ", orderKind:" + std::to_string(orderKind) +
        //        ", functionCode:" + std::to_string(functionCode) + ", askOrderID:" + std::to_string(askOrderID) +
        //        ", bidOrderID:" + std::to_string(bidOrderID) + ", channelId:" + std::to_string(channelId);
    }
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

    std::string ToString() const
    {
        auto updateTimeMillisec = updateTimeSpan % 1000000 / 1000;
        char updateTime[9];
        sprintf(updateTime,
                "%d:%d:%d",
                (updateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
                updateTimeSpan / 1000000 / 60 % 60,
                updateTimeSpan / 1000000 % 60);

        auto refUpdateTimeMicrosec = refUpdateTimeSpan % 1000000;
        char refUpdateTime[9];
        sprintf(refUpdateTime,
                "%d:%d:%d",
                (refUpdateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
                refUpdateTimeSpan / 1000000 / 60 % 60,
                refUpdateTimeSpan / 1000000 % 60);

        return std::string("instrumentId:") + instrumentId + ", updateTimeSpan:" + std::to_string(updateTimeSpan) +
               "(" + updateTime + "  " + std::to_string(updateTimeMillisec) + ")" +
               ", refUpdateTimeSpan:" + std::to_string(refUpdateTimeSpan) + "(" + refUpdateTime + " " +
               std::to_string(refUpdateTimeMicrosec) + ")" + ", orderPrice:" + std::to_string(orderPrice) +
               ", orderVolume:" + std::to_string(orderVolume) + ", isBuy:" + std::to_string(isBuy) +
               ", askOrderID:" + std::to_string(askOrderID) + ", bidOrderID:" + std::to_string(bidOrderID) +
               ", isCancel:" + std::to_string(isCancel);
    }
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

    int64_t askVolume5 = 0, askVolume4 = 0, askVolume3 = 0, askVolume2 = 0, askVolume1 = 0; // 卖5，卖4，卖3，卖2，卖1盘口挂单量
    int64_t bidVolume1 = 0, bidVolume2 = 0, bidVolume3 = 0, bidVolume4 = 0, bidVolume5 = 0; // 买1，买2，买3，买4，买5盘口挂单量

    int64_t openInterest = 0; // 忽略，不要管，无需关注

    double upperLimitPrice = 0; // 涨停板价
    double lowerLimitPrice = 0; // 跌停板价
    double highestPrice = 0;    // 最高价
    double lowestPrice = 0;     // 最低价
    double preClosePrice = 0;   // 昨天收盘价

    std::string ToString() const
    {
        return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay +
               ", updateTime:" + updateTime + ", updateMillisec:" + std::to_string(updateMillisec) +
               ", refUpdateTime:" + refUpdateTime + ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) +
               ", lastPrice:" + std::to_string(lastPrice) + ", volume:" + std::to_string(volume) +
               ", lastVolume:" + std::to_string(lastVolume) + ", turnover:" + std::to_string(turnover) +
               ", lastTurnover:" + std::to_string(lastTurnover) + ", askPrice5:" + std::to_string(askPrice5) +
               ", askPrice4:" + std::to_string(askPrice4) + ", askPrice3:" + std::to_string(askPrice3) +
               ", askPrice2:" + std::to_string(askPrice2) + ", askPrice1:" + std::to_string(askPrice1) +
               ", bidPrice1:" + std::to_string(bidPrice1) + ", bidPrice2:" + std::to_string(bidPrice2) +
               ", bidPrice3:" + std::to_string(bidPrice3) + ", bidPrice4:" + std::to_string(bidPrice4) +
               ", bidPrice5:" + std::to_string(bidPrice5) + ", askVolume5:" + std::to_string(askVolume5) +
               ", askVolume4:" + std::to_string(askVolume4) + ", askVolume3:" + std::to_string(askVolume3) +
               ", askVolume2:" + std::to_string(askVolume2) + ", askVolume1:" + std::to_string(askVolume1) +
               ", bidVolume1:" + std::to_string(bidVolume1) + ", bidVolume2:" + std::to_string(bidVolume2) +
               ", bidVolume3:" + std::to_string(bidVolume3) + ", bidVolume4:" + std::to_string(bidVolume4) +
               ", bidVolume5:" + std::to_string(bidVolume5) + ", openInterest:" + std::to_string(openInterest) +
               ", upperLimitPrice:" + std::to_string(upperLimitPrice) +
               ", lowerLimitPrice:" + std::to_string(lowerLimitPrice) +
               ", highestPrice:" + std::to_string(highestPrice) + ", lowestPrice:" + std::to_string(lowestPrice) +
               ", preClosePrice:" + std::to_string(preClosePrice);
    };
};

// inline std::string RawOrder::ToString() const
// {
//     return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay +
//             ", updateTime:" + updateTime + ", updateMillisec:" + std::to_string(updateMillisec) +
//             ", refUpdateTime:" + refUpdateTime + ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) +
//             ", orderSysID:" + std::to_string(orderSysID) + ", orderPrice:" + std::to_string(orderPrice) +
//             ", orderVolume:" + std::to_string(orderVolume) + ", direction:" + direction +
//             ", orderType:" + orderType + ", channelId:" + std::to_string(channelId);
// }

inline void Order::FromRawOrder(const RawOrder &rawOrder)
{
    orderID = rawOrder.orderSysID;

    // 解析updateTimeSpan
    std::tm timeInfo = {};
    strptime(rawOrder.updateTime, "%H:%M:%S", &timeInfo); // 解析时间
    strptime(rawOrder.tradingDay, "%Y%m%d", &timeInfo);   // 解析日期
    // 转换为自1970年1月1日以来的秒数
    time_t timeInSeconds = std::mktime(&timeInfo);
    // 转换为微妙级时间戳
    updateTimeSpan = static_cast<int64_t>(timeInSeconds) * 1000000 + rawOrder.updateMillisec * 1000;

    // 解析refUpdateTimeSpan
    strptime(rawOrder.refUpdateTime, "%H:%M:%S", &timeInfo); // 解析时间
    // strptime(rawOrder.tradingDay, "%Y%m%d", &timeInfo2);      // 解析日期
    // 转换为自1970年1月1日以来的秒数
    timeInSeconds = std::mktime(&timeInfo);
    refUpdateTimeSpan = static_cast<int64_t>(timeInSeconds) * 1000000 + rawOrder.refUpdateMicrosec;
    // std::cout << "timeInSeconds:" << timeInSeconds << ", timeInSeconds2:" << timeInSeconds2 << std::endl;
    // std::cout << "updateTime:" << rawOrder.updateTime << ", refUpdateTimeSpan:" << rawOrder.refUpdateTime << std::endl;
    // std::cout << "updateTime:" << rawOrder.updateMillisec << ", refUpdateTimeSpan:" << rawOrder.refUpdateMicrosec << std::endl;
    // std::cout << "updateTimeSpan:" << updateTimeSpan << ", refUpdateTimeSpan:" << refUpdateTimeSpan << std::endl;

    orderPrice = rawOrder.orderPrice;
    orderVolume = rawOrder.orderVolume;
    isBuy = rawOrder.direction == '1';
}

inline void Transaction::FromRawTransaction(const RawTransaction &rawTransaction)
{
    memcpy(instrumentId, rawTransaction.instrumentId, sizeof(instrumentId));

    // 解析updateTimeSpan
    std::tm timeInfo = {};
    strptime(rawTransaction.updateTime, "%H:%M:%S", &timeInfo); // 解析时间
    strptime(rawTransaction.tradingDay, "%Y%m%d", &timeInfo);   // 解析日期
    // 转换为自1970年1月1日以来的秒数
    time_t timeInSeconds = std::mktime(&timeInfo);
    // 转换为微妙级时间戳
    updateTimeSpan = static_cast<int64_t>(timeInSeconds * 1000000 + rawTransaction.updateMillisec * 1000);

    // 解析refUpdateTimeSpan
    strptime(rawTransaction.refUpdateTime, "%H:%M:%S", &timeInfo); // 解析时间
    timeInSeconds = std::mktime(&timeInfo);
    refUpdateTimeSpan = static_cast<int64_t>(timeInSeconds * 1000000 + rawTransaction.refUpdateMicrosec);

    orderPrice = rawTransaction.tradePrice;
    orderVolume = rawTransaction.tradeVolume;
    isBuy = rawTransaction.direction == '1';
    bidOrderID = rawTransaction.bidOrderID;
    askOrderID = rawTransaction.askOrderID;
    isCancel = rawTransaction.functionCode == 'C';
}

// inline std::string Order::ToString() const
// {
//     auto updateTimeMillisec = updateTimeSpan % 1000000 / 1000;
//     char updateTime[9];
//     sprintf(updateTime,
//             "%d:%d:%d",
//             (updateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
//             updateTimeSpan / 1000000 / 60 % 60,
//             updateTimeSpan / 1000000 % 60);

//     auto refUpdateTimeMicrosec = refUpdateTimeSpan % 1000000;
//     char refUpdateTime[9];
//     sprintf(refUpdateTime,
//             "%d:%d:%d",
//             (refUpdateTimeSpan / 1000000 / 60 / 60 + 8) % 24,
//             refUpdateTimeSpan / 1000000 / 60 % 60,
//             refUpdateTimeSpan / 1000000 % 60);

//     return std::string("orderID:") + std::to_string(orderID) + ", updateTimeSpan:" + std::to_string(updateTimeSpan) +
//            "(" + updateTime + "  " + std::to_string(updateTimeMillisec) + ")" +
//            ", refUpdateTimeSpan:" + std::to_string(refUpdateTimeSpan) + "(" + refUpdateTime + " " +
//            std::to_string(refUpdateTimeMicrosec) + ")" + ", orderPrice:" + std::to_string(orderPrice) +
//            ", orderVolume:" + std::to_string(orderVolume) + ", isBuy:" + std::to_string(isBuy);
// }

// inline void Transaction::FromRawTransaction(const RawTransaction &rawTransaction) {

// }
