#include <system_error>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

#include "logger.h"
#include "transaction_mgr.h"
#include "live_tarding_data_simulator.h"
#include "snapshot_checker.h"

int main()
{
    // 获取当前时间
    std::time_t beginTime = std::time(0);
    std::tm *beginLocalTime = std::localtime(&beginTime);

    // 打印当前时间，格式为小时:分钟:秒
    std::cout << "BeginTime: " << std::setw(2) << std::setfill('0') << beginLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << beginLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0')
              << beginLocalTime->tm_sec << std::endl;

    // 初始化交易管理器
    TransactionMgr transMgr;
    transMgr.SetPreClosePrice(168.998);
    
    // 初始化交易数据模拟器，并注册order transaction事件处理函数
    LiveTradingDataSimulator liveTradingDataSimulator;
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_ORDER,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveOrder(data); }));
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_TRANSACTION,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveTransaction(data); }));


    std::cout << "DiffTime1: " << std::time(0) - beginTime << "s" << std::endl;

    // 初始化实盘数据模拟器
    int ret = liveTradingDataSimulator.Init();
    if (ret)
    {
        LOG_ERROR1("Init liveTradingDataSimulator error!");
        return ret;
    }
    
    std::cout << "DiffTime2: " << std::time(0) - beginTime << "s" << std::endl;

    // 运行实盘数据模拟器
    liveTradingDataSimulator.Run();

    std::cout << "DiffTime3: " << std::time(0) - beginTime << "s" << std::endl;

    // 导出生成tick数据
    transMgr.DumpSnapshotToFile("data/GenTick.csv");


    std::cout << "DiffTime4: " << std::time(0) - beginTime << "s" << std::endl;

    // 检查tick数据是否正确率
    TickDataChecker::CheckTickDataExistPercent("data/Tick.csv", transMgr.GetSnapshot());

    // 获取结束时间
    std::time_t endTime = std::time(0);
    std::tm *endLocalTime = std::localtime(&endTime);
    
    // 打印结束时间，格式为小时:分钟:秒
    std::cout << "EndTime: " << std::setw(2) << std::setfill('0') << endLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << endLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0')
              << endLocalTime->tm_sec << std::endl;


    std::cout << "DiffTime: " << endTime - beginTime << "s" << std::endl;

    return 0;
}