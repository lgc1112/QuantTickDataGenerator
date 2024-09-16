/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2024-09-16 12:12:11
 * @edit: regangcli
 * @brief: 
 */

#include "logger.h"

// 当前日志级别
LogLevel currentLogLevel = LOG_DEBUG; // 默认为最详细的级别

// 检查日志级别是否足够高以输出日志
bool isLogLevelEnabled(LogLevel level) {
    return level >= currentLogLevel;
}

void log(LogLevel level, const char* file, const char* function, int line, const char* format, ...) 
{
    // 定义日志级别字符串
    const char* levelStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

    // 检查日志级别
    if (!(level >= currentLogLevel)) {
        return;
    }

    // 获取当前时间
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);

    // 构建日志消息
    char timeBuffer[100];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tm_info);

    char messageBuffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    va_end(args);

    // 输出日志
    std::cout << timeBuffer << " [" << levelStrings[level] << "] [" << file << ":" << line << " " << function << "] " << messageBuffer << std::endl;
}

