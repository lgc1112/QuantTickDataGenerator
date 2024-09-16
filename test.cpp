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
#include "live_tarding_data_simulator.h"
#include "logger.h"

int main()
{
    // 获取当前时间
    std::time_t beginTime = std::time(0);
    std::tm *beginLocalTime = std::localtime(&beginTime);

    // 打印当前时间，格式为小时:分钟:秒
    std::cout << "BeginTime: " << std::setw(2) << std::setfill('0') << beginLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << beginLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0')
              << beginLocalTime->tm_sec << std::endl;

    // 初始化交易管理器并注册事件处理函数
    LiveTradingDataSimulator liveTradingDataSimulator;
    TransactionMgr transMgr;
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_ORDER,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveOrder(data); }));
    liveTradingDataSimulator.RegisterEventHandler(
        DataEventType::RECV_TRANSACTION,
        std::function<void(void *)>([&transMgr](void *data) { transMgr.OnReceiveTransaction(data); }));

    // 初始化实盘数据模拟器
    int ret = liveTradingDataSimulator.Init();
    if (ret)
    {
        LOG_ERROR1("Init liveTradingDataSimulator error!");
        return ret;
    }

    // 启动实盘数据模拟器
    liveTradingDataSimulator.Run();


    // 生成tick数据
    std::cout << "GenTickNum: " << transMgr.GetSnapshotNum() << std::endl; 
    transMgr.DumpSnapshotToFile("data/GenTick.csv");

    // 获取当前时间
    std::time_t endTime = std::time(0);
    std::tm *endLocalTime = std::localtime(&endTime);
    // 打印当前时间，格式为小时:分钟:秒
    std::cout << "EndTime: " << std::setw(2) << std::setfill('0') << endLocalTime->tm_hour << ":" << std::setw(2)
              << std::setfill('0') << endLocalTime->tm_min << ":" << std::setw(2) << std::setfill('0')
              << endLocalTime->tm_sec << std::endl;

    // std::cout << "DiffTime: " << endTime - beginTime << "s" << std::endl;

    return 0;
}