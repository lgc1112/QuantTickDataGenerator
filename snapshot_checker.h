/*
 * @file: 
 * @Author: ligengchao
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-17 17:35:40
 * @edit: ligengchao
 * @brief: 
 */

#pragma once

#include <unordered_map>
#include <vector>

#include "data_def.h"

class SnapshotChecker
{
public:
    SnapshotChecker() = default;
    ~SnapshotChecker() = default;

    // 检查Tick存在百分比
    static double CheckTickDataExistPercent(const std::string &filename,
                                            const std::vector<RawSnapshot> &snapshots);
};
