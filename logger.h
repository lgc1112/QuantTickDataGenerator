/*
 * @file: 
 * @Author: ligengchao
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 12:12:15
 * @edit: ligengchao
 * @brief: 
 */
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdarg>

// 定义日志级别枚举
enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

// 日志打印函数
void log(LogLevel level, const char* file, const char* function, int line, const char* format, ...);

// 日志宏，使用fmt::format和fmt::make_format_args
#define LOG(level, format, ...) \
    log(level, __FILE__, __func__, __LINE__, format, __VA_ARGS__)

#define LOG_DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(LOG_INFO, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_WARN, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(LOG_ERROR, fmt, __VA_ARGS__)

#define LOG_DEBUG1(str) LOG(LOG_DEBUG, "%s", str)
#define LOG_INFO1(str) LOG(LOG_INFO, "%s", str)
#define LOG_WARN1(str) LOG(LOG_WARN, "%s", str)
#define LOG_ERROR1(str) LOG(LOG_ERROR, "%s", str)
