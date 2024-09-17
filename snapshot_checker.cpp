/*
 * @file: 
 * @Author: ligengchao
 * @copyright: 
 * @Date: 2024-09-17 17:35:56
 * @edit: ligengchao
 * @brief: 
 */

#include <iostream>
#include <fstream>
#include <system_error>
#include <cstring>

#include "snapshot_checker.h"
#include "logger.h"

double TickDataChecker::CheckTickDataExistPercent(const std::string &filename,
                                                  const std::vector<RawSnapshot> &snapshots)
{
    // 构建快照map {volume : snapshots}
    std::unordered_map<int64_t, std::vector<const RawSnapshot *>> snapshotsMap;
    for (const auto &snapshot : snapshots)
        snapshotsMap[snapshot.volume].push_back(&snapshot);

    std::ifstream file(filename);
    std::string line;
    if (!file)
    {
        std::error_code ec(errno, std::generic_category());
        LOG_ERROR("Error opening file %s, err:%s'", filename.c_str(), ec.message().c_str());
        return 0;
    }

    RawSnapshot rawSnapshot;
    std::string temps;
    char tmpc;
    int totalCount = 0, successCount = 0;

    // 跳过前45行
    int currentLine = 1;
    while (currentLine++ <= 45 && std::getline(file, line))
        ;

    // 读取到4799行
    while (std::getline(file, line) && currentLine++ <= 4800)
    {
        totalCount++;

        std::stringstream ss(line);

        std::getline(ss, temps, ',');
        // strncpy(rawSnapshot.instrumentId, temps.c_str(), sizeof(rawSnapshot.instrumentId) - 1);
        // rawSnapshot.instrumentId[sizeof(rawSnapshot.instrumentId) - 1] = '\0';

        std::getline(ss, temps, ',');
        // strncpy(rawSnapshot.tradingDay, temps.c_str(), sizeof(rawSnapshot.tradingDay) - 1);
        // rawSnapshot.tradingDay[sizeof(rawSnapshot.tradingDay) - 1] = '\0';

        std::getline(ss, temps, ',');
        strncpy(rawSnapshot.updateTime, temps.c_str(), sizeof(rawSnapshot.updateTime) - 1);
        rawSnapshot.updateTime[sizeof(rawSnapshot.updateTime) - 1] = '\0';
        ss >> rawSnapshot.updateMillisec >> tmpc;

        std::getline(ss, temps, ',');
        // strncpy(rawSnapshot.refUpdateTime, temps.c_str(), sizeof(rawSnapshot.refUpdateTime) - 1);
        // rawSnapshot.refUpdateTime[sizeof(rawSnapshot.refUpdateTime) - 1] = '\0';
        ss >> rawSnapshot.refUpdateMicrosec >> tmpc;

        ss >> rawSnapshot.lastPrice >> tmpc;
        ss >> rawSnapshot.volume >> tmpc;
        ss >> rawSnapshot.lastVolume >> tmpc;
        ss >> rawSnapshot.turnover >> tmpc;
        ss >> rawSnapshot.lastTurnover >> tmpc;
        ss >> rawSnapshot.askPrice5 >> tmpc;
        ss >> rawSnapshot.askPrice4 >> tmpc;
        ss >> rawSnapshot.askPrice3 >> tmpc;
        ss >> rawSnapshot.askPrice2 >> tmpc;
        ss >> rawSnapshot.askPrice1 >> tmpc;
        ss >> rawSnapshot.bidPrice1 >> tmpc;
        ss >> rawSnapshot.bidPrice2 >> tmpc;
        ss >> rawSnapshot.bidPrice3 >> tmpc;
        ss >> rawSnapshot.bidPrice4 >> tmpc;
        ss >> rawSnapshot.bidPrice5 >> tmpc;
        ss >> rawSnapshot.askVolume5 >> tmpc;
        ss >> rawSnapshot.askVolume4 >> tmpc;
        ss >> rawSnapshot.askVolume3 >> tmpc;
        ss >> rawSnapshot.askVolume2 >> tmpc;
        ss >> rawSnapshot.askVolume1 >> tmpc;
        ss >> rawSnapshot.bidVolume1 >> tmpc;
        ss >> rawSnapshot.bidVolume2 >> tmpc;
        ss >> rawSnapshot.bidVolume3 >> tmpc;
        ss >> rawSnapshot.bidVolume4 >> tmpc;
        ss >> rawSnapshot.bidVolume5 >> tmpc;
        ss >> rawSnapshot.openInterest >> tmpc;
        ss >> rawSnapshot.upperLimitPrice >> tmpc;
        ss >> rawSnapshot.lowerLimitPrice >> tmpc;
        ss >> rawSnapshot.highestPrice >> tmpc;
        ss >> rawSnapshot.lowestPrice >> tmpc;
        ss >> rawSnapshot.preClosePrice >> tmpc;

        auto it = snapshotsMap.find(rawSnapshot.volume);
        if (it == snapshotsMap.end())
        {
            LOG_WARN("snapshot not found, volume: %lld", rawSnapshot.volume);
            continue;
        }

        bool success = false;
        for (auto &&snapShort : it->second)
        {
            if (rawSnapshot.Compare(*snapShort))
            {
                successCount++;
                success = true;
                break;
            }
        }

        if (!success)
            LOG_WARN("snapshot not found, volume: %lld, %s", rawSnapshot.volume, rawSnapshot.ToString().c_str());

    }

    // 打印匹配情况
    std::cout << "GenTickNum: " << snapshots.size() << "        TickNum:" << totalCount
              << "        MatchNum:" << successCount << "        NoMatchNum:" << totalCount - successCount
              << "        MatchRate:" << 1.0 * successCount / totalCount * 100 << "%" << std::endl;

    LOG_INFO("successCount: %d, totalCount: %d, success percent: %f%",
             successCount,
             totalCount,
             1.0 * successCount / totalCount * 100);

    return 0;
}