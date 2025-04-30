#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>
#include <iomanip>
#include <chrono>

namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    template<typename... Args>
    static void log(LogLevel level, const char* format, Args... args) {
        getInstance().logImpl(level, format, args...);
    }

    static void setLogLevel(LogLevel level) {
        getInstance().currentLevel = level;
    }

private:
    Logger() : currentLevel(LogLevel::INFO) {}
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel currentLevel;
    std::mutex logMutex;

    template<typename... Args>
    void logImpl(LogLevel level, const char* format, Args... args) {
        if (level < currentLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);
        std::stringstream ss;
        
        // Add timestamp
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " ";
        
        // Add log level
        switch (level) {
            case LogLevel::DEBUG:   ss << "[DEBUG] "; break;
            case LogLevel::INFO:    ss << "[INFO] "; break;
            case LogLevel::WARNING: ss << "[WARNING] "; break;
            case LogLevel::ERROR:   ss << "[ERROR] "; break;
            case LogLevel::FATAL:   ss << "[FATAL] "; break;
        }

        // Format the message
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), format, args...);
        ss << buffer << std::endl;

        std::cout << ss.str();
    }
};

} // namespace utils

// Define macros outside the namespace to make them globally accessible
#define LOG_DEBUG(...)   utils::Logger::log(utils::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)    utils::Logger::log(utils::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) utils::Logger::log(utils::LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...)   utils::Logger::log(utils::LogLevel::ERROR, __VA_ARGS__)
#define LOG_FATAL(...)   utils::Logger::log(utils::LogLevel::FATAL, __VA_ARGS__) 