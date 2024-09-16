#include <climits>
#include <cstdio>
#include <ios>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <system_error>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

#include "transaction_mgr.h"
#include "logger.h"

// 函数用于从CSV文件中读取RawOrder数据
void ReadRawOrders(const std::string &filename, std::vector<Order> &orders)
{
    std::ifstream file(filename);
    std::string line;
    // file.open(filename);
    if (!file)
    {
        std::error_code ec(errno, std::generic_category());
        LOG_ERROR("Error opening file %s, err:%s'", filename.c_str(), ec.message().c_str());
        return;
    }

    RawOrder rawOrder;
    Order order;
    std::string temps;
    char tmpc;
    int count = 0;
    std::getline(file, line); // 跳过第一行
    while (std::getline(file, line))
    {
        // LOG_DEBUG("rawLine: %s", line.c_str());
        std::stringstream ss(line);
        
        std::getline(ss, temps, ',');
        strncpy(rawOrder.instrumentId, temps.c_str(), sizeof(rawOrder.instrumentId) - 1);
        rawOrder.instrumentId[sizeof(rawOrder.instrumentId) - 1] = '\0';

        std::getline(ss, temps, ',');
        strncpy(rawOrder.tradingDay, temps.c_str(), sizeof(rawOrder.tradingDay) - 1);
        rawOrder.tradingDay[sizeof(rawOrder.tradingDay) - 1] = '\0';

        std::getline(ss, temps, ',');
        strncpy(rawOrder.updateTime, temps.c_str(), sizeof(rawOrder.updateTime) - 1);
        rawOrder.updateTime[sizeof(rawOrder.updateTime) - 1] = '\0';

        ss >> rawOrder.updateMillisec >> tmpc;

        std::getline(ss, temps, ',');
        strncpy(rawOrder.refUpdateTime, temps.c_str(), sizeof(rawOrder.refUpdateTime) - 1);
        rawOrder.refUpdateTime[sizeof(rawOrder.refUpdateTime) - 1] = '\0';
        
        ss >> rawOrder.refUpdateMicrosec >> tmpc;
        ss >> rawOrder.orderSysID >> tmpc;
        ss >> rawOrder.orderPrice >> tmpc;
        ss >> rawOrder.orderVolume >> tmpc;
        ss >> rawOrder.direction >> tmpc;
        ss >> rawOrder.orderType >> tmpc;
        ss >> rawOrder.channelId >> tmpc;

        order.FromRawOrder(rawOrder);
        orders.push_back(order);

        if (++count > 10)
            break;
        // LOG_DEBUG("Read raw order:%s", rawOrder.ToString().c_str());
        // LOG_DEBUG("Read order:%s", order.ToString().c_str());
    }

    file.close();
}

// 函数用于从CSV文件中读取RawTransaction数据
void ReadRawTransactions(const std::string &filename, std::vector<Transaction> &transactions)
{
    std::ifstream file(filename);
    std::string line;
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    RawTransaction rawTransaction;
    Transaction transaction;
    std::string temps;
    char tmpc;
    int count = 0;
    std::getline(file, line); // 跳过第一行
    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::getline(ss, temps, ',');
        strncpy(rawTransaction.instrumentId, temps.c_str(), sizeof(rawTransaction.instrumentId) - 1);
        rawTransaction.instrumentId[sizeof(rawTransaction.instrumentId) - 1] = '\0';

        std::getline(ss, temps, ',');
        strncpy(rawTransaction.tradingDay, temps.c_str(), sizeof(rawTransaction.tradingDay) - 1);
        rawTransaction.tradingDay[sizeof(rawTransaction.tradingDay) - 1] = '\0';

        std::getline(ss, temps, ',');
        strncpy(rawTransaction.updateTime, temps.c_str(), sizeof(rawTransaction.updateTime) - 1);
        rawTransaction.updateTime[sizeof(rawTransaction.updateTime) - 1] = '\0';

        ss >> rawTransaction.updateMillisec >> tmpc;

        std::getline(ss, temps, ',');
        strncpy(rawTransaction.refUpdateTime, temps.c_str(), sizeof(rawTransaction.refUpdateTime) - 1);
        rawTransaction.refUpdateTime[sizeof(rawTransaction.refUpdateTime) - 1] = '\0';
        
        ss >> rawTransaction.refUpdateMicrosec >> tmpc;
        ss >> rawTransaction.tradeId >> tmpc;
        ss >> rawTransaction.tradePrice >> tmpc;
        ss >> rawTransaction.tradeVolume >> tmpc;
        ss >> rawTransaction.turnover >> tmpc;
        ss >> rawTransaction.direction >> tmpc;
        ss >> rawTransaction.orderKind >> tmpc;
        ss >> rawTransaction.functionCode >> tmpc;
        ss >> rawTransaction.askOrderID >> tmpc;
        ss >> rawTransaction.bidOrderID >> tmpc;
        ss >> rawTransaction.channelId >> tmpc;

        transaction.FromRawTransaction(rawTransaction);
        transactions.push_back(transaction);

        if (++count > 10)
            break;

        // LOG_DEBUG("Read raw transaction:%s", rawTransaction.ToString().c_str());
        // LOG_DEBUG("Read transaction:%s", transaction.ToString().c_str());

    }
    file.close();
}

enum DataEventType
{
    RECV_ORDER = 0,
    RECV_TRANSACTION,
    END,
};

// 实盘数据模拟器
class LiveTradingDataSimulator
{
public:
    LiveTradingDataSimulator() { }
    ~LiveTradingDataSimulator() { }

    // 初始化函数
    int Init();

    // 初始化函数
    void Run();

    //
    // 注册事件处理器
    void RegisterEventHandler(DataEventType event, std::function<void(void *)> handler);

private:
    // int curOrderIdx = 0;
    // int curTransactionIdx = 0;
    // 事件处理器列表
    std::array<std::vector<std::function<void(void *)>>, DataEventType::END> eventHandlers_;

    std::vector<Order> orders_;
    std::vector<Transaction> transactions_;
};

int LiveTradingDataSimulator::Init()
{
    // // 创建后台线程读取Order数据
    // std::thread orderThread(ReadRawOrders, "data/Orders.csv", std::ref(orders_));
    // // 创建后台线程读取Transaction数据
    // std::thread transactionThread(ReadRawTransactions, "data/Trans.csv", std::ref(transactions_));

    // // 等待线程完成
    // orderThread.join();
    // transactionThread.join();
    ReadRawOrders("data/Orders.csv", std::ref(orders_));
    ReadRawTransactions("data/Trans.csv", std::ref(transactions_));
    int orderSize = orders_.size(), transactionSize = transactions_.size();
    std::cout << "Order: " << orderSize << std::endl;
    std::cout << "Transaction: " << transactionSize << std::endl;

    return 0;
}

inline void LiveTradingDataSimulator::Run()
{
    int orderSize = orders_.size(), transactionSize = transactions_.size();
    int curOrderIdx = 0, curTransactionIdx = 0;
    // int count = 0;
    // 创建网络线程模拟读取交易所数据
    // 模拟实盘情况按照本地消息接收时间(RefUpdateTime + RefUpdateMillisec)顺序处理每一条Order或Transaction
    while (curOrderIdx < orderSize || curTransactionIdx < transactionSize)
    {
        // if (++count == 10)
        //     exit(0);

        if (curOrderIdx < orderSize && curTransactionIdx < transactionSize)
        {
            auto latestOrder = orders_[curOrderIdx];
            auto latestTransaction = transactions_[curTransactionIdx];
            // 收到一条新的order
            if (latestOrder.refUpdateTimeSpan < latestTransaction.refUpdateTimeSpan)
            {
                // std::cout << "Recv Order: " << latestOrder.ToString() << std::endl;
                // LOG_INFO( "Recv Order: %s", latestOrder.ToString().c_str());
                ++curOrderIdx;
                for (auto &handler : eventHandlers_[DataEventType::RECV_ORDER])
                    handler(&latestOrder);
            }
            // 收到一条新的transaction
            else
            {
                // std::cout << "Recv Transaction: " << latestTransaction.ToString() << std::endl;
                // LOG_INFO( "Recv Transaction: %s", latestTransaction.ToString().c_str());
                ++curTransactionIdx;
                for (auto &handler : eventHandlers_[DataEventType::RECV_TRANSACTION])
                    handler(&latestTransaction);
            }

            continue;
        }

        // 收到一条新的order
        if (curOrderIdx < orderSize)
        {
            auto latestOrder = orders_[curOrderIdx++];
            // std::cout << "Recv Order: " << latestOrder.ToString() << std::endl;
            // LOG_INFO( "Recv Order: %s", latestOrder.ToString().c_str());
            for (auto &handler : eventHandlers_[DataEventType::RECV_ORDER])
                handler(&latestOrder);

            continue;
        }

        // 收到一条新的transaction
        if (curTransactionIdx < transactionSize)
        {
            auto latestTransaction = transactions_[curTransactionIdx++];
            // std::cout << "Recv Transaction: " << latestTransaction.ToString() << std::endl;
            // LOG_INFO( "Recv Transaction: %s", latestTransaction.ToString().c_str());
            for (auto &handler : eventHandlers_[DataEventType::RECV_TRANSACTION])
                handler(&latestTransaction);
        }
    }
}

inline void LiveTradingDataSimulator::RegisterEventHandler(DataEventType event, std::function<void(void *)> handler)
{
    eventHandlers_[event].push_back(handler);
}

class SnapShortMgr
{
};

int main()
{   
    // 获取当前时间
    std::time_t beginTime = std::time(0);
    std::tm *beginLocalTime = std::localtime(&beginTime);
    // 打印当前时间，格式为小时:分钟:秒
    // 手动格式化时间
    std::cout << "BeginTime: " << std::setw(2) << std::setfill('0') << beginLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << beginLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0') << beginLocalTime->tm_sec
              << std::endl;

    LiveTradingDataSimulator liveTradingDataSimulator;
    TransactionMgr transMgr;
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_ORDER,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveOrder(data); }));
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_TRANSACTION,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveTransaction(data); }));

    int ret = liveTradingDataSimulator.Init();
    if (ret)
    {
        printf("Init liveTradingDataSimulator error!\n");
        return ret;
    }

    liveTradingDataSimulator.Run();

    std::time_t endTime = std::time(0);
    std::tm *endLocalTime = std::localtime(&endTime);
    // 打印当前时间，格式为小时:分钟:秒
    // 手动格式化时间
    std::cout << "EndTime: " << std::setw(2) << std::setfill('0') << endLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << endLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0') << endLocalTime->tm_sec
              << std::endl;

              
    std::cout << "DiffTime: " << endTime - beginTime << "s" << std::endl;

    return 0;
}