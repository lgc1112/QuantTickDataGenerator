/*
 * @file: 
 * @Author: ligengchao
 * @copyright: 
 * @Date: 2024-09-17 17:35:40
 * @edit: ligengchao
 * @brief: 
 */

#pragma once

#include <unordered_map>
#include <vector>

#include "data_def.h"


// Checker检查器
class TickDataChecker
{
#pragma region : 对外 api
public:
    // 检查Tick存在百分比
    static double CheckTickDataExistPercent(const std::string &filename,
                                            const std::vector<RawSnapshot> &snapshots);
#pragma endregion
};
