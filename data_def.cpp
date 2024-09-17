/*
 * @file: 
 * @Author: ligengchao
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-17 13:26:52
 * @edit: ligengchao
 * @brief: 
 */
#include <cmath>

#include "data_def.h"
#include "logger.h"


bool OrderCompare::operator()(const Order *a, const Order *b) const
{
    return a->orderPrice < b->orderPrice;
}

void Order::FromRawOrder(const RawOrder &rawOrder)
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

std::string Order::ToString() const
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

    return std::string("orderID:") + std::to_string(orderID) + ", updateTimeSpan:" + std::to_string(updateTimeSpan) +
           "(" + updateTime + "  " + std::to_string(updateTimeMillisec) + ")" +
           ", refUpdateTimeSpan:" + std::to_string(refUpdateTimeSpan) + "(" + refUpdateTime + " " +
           std::to_string(refUpdateTimeMicrosec) + ")" + ", orderPrice:" + std::to_string(orderPrice) +
           ", orderVolume:" + std::to_string(orderVolume) + ", isBuy:" + std::to_string(isBuy);
}

void Transaction::FromRawTransaction(const RawTransaction &rawTransaction)
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

std::string Transaction::ToString() const
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

    return std::string("instrumentId:") + instrumentId + ", updateTimeSpan:" + std::to_string(updateTimeSpan) + "(" +
           updateTime + "  " + std::to_string(updateTimeMillisec) + ")" +
           ", refUpdateTimeSpan:" + std::to_string(refUpdateTimeSpan) + "(" + refUpdateTime + " " +
           std::to_string(refUpdateTimeMicrosec) + ")" + ", orderPrice:" + std::to_string(orderPrice) +
           ", orderVolume:" + std::to_string(orderVolume) + ", isBuy:" + std::to_string(isBuy) +
           ", askOrderID:" + std::to_string(askOrderID) + ", bidOrderID:" + std::to_string(bidOrderID) +
           ", isCancel:" + std::to_string(isCancel);
}

std::string RawSnapshot::ToString() const
{
    return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay + ", updateTime:" + updateTime +
           ", updateMillisec:" + std::to_string(updateMillisec) + ", refUpdateTime:" + refUpdateTime +
           ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) + ", lastPrice:" + std::to_string(lastPrice) +
           ", volume:" + std::to_string(volume) + ", lastVolume:" + std::to_string(lastVolume) +
           ", turnover:" + std::to_string(turnover) + ", lastTurnover:" + std::to_string(lastTurnover) +
           ", askPrice5:" + std::to_string(askPrice5) + ", askPrice4:" + std::to_string(askPrice4) +
           ", askPrice3:" + std::to_string(askPrice3) + ", askPrice2:" + std::to_string(askPrice2) +
           ", askPrice1:" + std::to_string(askPrice1) + ", bidPrice1:" + std::to_string(bidPrice1) +
           ", bidPrice2:" + std::to_string(bidPrice2) + ", bidPrice3:" + std::to_string(bidPrice3) +
           ", bidPrice4:" + std::to_string(bidPrice4) + ", bidPrice5:" + std::to_string(bidPrice5) +
           ", askVolume5:" + std::to_string(askVolume5) + ", askVolume4:" + std::to_string(askVolume4) +
           ", askVolume3:" + std::to_string(askVolume3) + ", askVolume2:" + std::to_string(askVolume2) +
           ", askVolume1:" + std::to_string(askVolume1) + ", bidVolume1:" + std::to_string(bidVolume1) +
           ", bidVolume2:" + std::to_string(bidVolume2) + ", bidVolume3:" + std::to_string(bidVolume3) +
           ", bidVolume4:" + std::to_string(bidVolume4) + ", bidVolume5:" + std::to_string(bidVolume5) +
           ", openInterest:" + std::to_string(openInterest) + ", upperLimitPrice:" + std::to_string(upperLimitPrice) +
           ", lowerLimitPrice:" + std::to_string(lowerLimitPrice) + ", highestPrice:" + std::to_string(highestPrice) +
           ", lowestPrice:" + std::to_string(lowestPrice) + ", preClosePrice:" + std::to_string(preClosePrice);
};

bool CompareDouble(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

bool RawSnapshot::Compare(const RawSnapshot &other) const {
    if (volume != other.volume || !CompareDouble(lastPrice, other.lastPrice) || !CompareDouble(turnover, other.turnover, 1) )
    {
        // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
        return false;
    }

    // if (askPrice1 != other.askPrice1 || askPrice2 != other.askPrice2 || askPrice3 != other.askPrice3 ||
    //     askPrice4 != other.askPrice4 || askPrice5 != other.askPrice5)
    // {
    //     // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
    //     return false;
    // }

    if (!CompareDouble(askPrice1, other.askPrice1) || !CompareDouble(askPrice2, other.askPrice2) ||
        !CompareDouble(askPrice3, other.askPrice3) || !CompareDouble(askPrice4, other.askPrice4) ||
        !CompareDouble(askPrice5, other.askPrice5))
    {
        // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
        return false;
    }

    // if (bidPrice1 != other.bidPrice1 || bidPrice2 != other.bidPrice2 || bidPrice3 != other.bidPrice3 ||
    //     bidPrice4 != other.bidPrice4 || bidPrice5 != other.bidPrice5)
    // {
    //     // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
    //     return false;
    // }

    if (!CompareDouble(bidPrice1, other.bidPrice1) || !CompareDouble(bidPrice2, other.bidPrice2) ||
        !CompareDouble(bidPrice3, other.bidPrice3) || !CompareDouble(bidPrice4, other.bidPrice4) ||
        !CompareDouble(bidPrice5, other.bidPrice5))
    {
        // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
        return false;
    }

    if (askVolume1 != other.askVolume1 || askVolume2 != other.askVolume2 || askVolume3 != other.askVolume3 ||
        askVolume4 != other.askVolume4 || askVolume5 != other.askVolume5)
    {
        // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
        return false;
    }

    if (bidVolume1 != other.bidVolume1 || bidVolume2 != other.bidVolume2 || bidVolume3 != other.bidVolume3 ||
        bidVolume4 != other.bidVolume4 || bidVolume5 != other.bidVolume5)
    {
        // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
        return false;
    }

    // if (highestPrice != other.highestPrice || lowestPrice != other.lowestPrice)
    // {
    //     // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
    //     return false;
    // }

    // if (!CompareDouble(upperLimitPrice, other.upperLimitPrice) || !CompareDouble(lowerLimitPrice, other.lowerLimitPrice))
    // {
    //     LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
    //     return false;
    // }

    // if (!CompareDouble(highestPrice, other.highestPrice) || !CompareDouble(lowestPrice, other.lowestPrice || !CompareDouble(preClosePrice, other.preClosePrice)))
    // {
    //     // LOG_WARN("self:%s, other:%s", ToString().c_str(), other.ToString().c_str());
    //     return false;
    // }

    return true;
}

std::string RawTransaction::ToString() const
{
    return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay + ", updateTime:" + updateTime +
           ", updateMillisec:" + std::to_string(updateMillisec) + ", refUpdateTime:" + refUpdateTime +
           ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) + ", tradeId:" + std::to_string(tradeId) +
           ", tradePrice:" + std::to_string(tradePrice) + ", tradeVolume:" + std::to_string(tradeVolume) +
           ", turnover:" + std::to_string(turnover) + ", direction:" + direction + ", orderKind:" + orderKind +
           ", functionCode:" + functionCode + ", askOrderID:" + std::to_string(askOrderID) +
           ", bidOrderID:" + std::to_string(bidOrderID) + ", channelId:" + std::to_string(channelId);
}

std::string RawOrder::ToString() const
{
    return std::string("instrumentId:") + instrumentId + ", tradingDay:" + tradingDay + ", updateTime:" + updateTime +
           ", updateMillisec:" + std::to_string(updateMillisec) + ", refUpdateTime:" + refUpdateTime +
           ", refUpdateMicrosec:" + std::to_string(refUpdateMicrosec) + ", orderSysID:" + std::to_string(orderSysID) +
           ", orderPrice:" + std::to_string(orderPrice) + ", orderVolume:" + std::to_string(orderVolume) +
           ", direction:" + direction + ", orderType:" + orderType + ", channelId:" + std::to_string(channelId);
}
