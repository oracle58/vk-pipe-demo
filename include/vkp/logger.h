#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <fmt/core.h>
#include <set>

enum class log_level {
    FATAL = 0,
    ERROR = 1,
    WARN  = 2,
    INFO  = 3,
    DEBUG = 4,
    TRACE = 5
};

class logger {
public:
    static void log(log_level level, const std::string& message, const char* file = __FILE__, int line = __LINE__) {
        const char* level_strings[] = {
            "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
        };

        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

        std::tm tm_buf{};
#ifdef _WIN32
        localtime_s(&tm_buf, &now_time_t);
#else
        localtime_r(&now_time_t, &tm_buf);
#endif

        std::stringstream time_stream;
        time_stream << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");

        std::stringstream time_stream_full;
        time_stream_full << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        time_stream_full << '.' << std::setfill('0') << std::setw(6) << now_us.count();

        std::filesystem::path full_path = std::filesystem::path(file).lexically_normal();
        std::string file_name = full_path.filename().string();

        // Recognized top-level project folders
        static const std::set<std::string> project_roots = {"src", "shaders"};
        std::string module = "unknown";

        for (const auto& part : full_path) {
            if (project_roots.contains(part.string())) {
                module = part.string();
                break;
            }
        }

        std::stringstream header;
        header << time_stream.str()
               << " " << level_strings[static_cast<int>(level)]
               << " [" << module << " " << file_name << ":" << line << "] "; {
            std::string full_message = fmt::format("{}{}", header.str(), message);
            std::lock_guard<std::mutex> lock(io_mutex_);
            std::cout << full_message << '\n';
        }

#ifndef _DEBUG
        std::lock_guard<std::mutex> lock(json_mutex_);
        std::filesystem::create_directories("engine/logs");
        std::ofstream out("engine/logs/log.txt", std::ios::app);
        out << time_stream_full.str() << " "
            << level_strings[static_cast<int>(level)] << " ["
            << module << " " << file_name << ":" << line << "] "
            << message << '\n';
#endif
    }

private:
    inline static std::mutex io_mutex_;
    inline static std::mutex json_mutex_;
};

#define LOG_FATAL(msg, ...) logger::log(log_level::FATAL, fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
#define LOG_ERROR(msg, ...) logger::log(log_level::ERROR, fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
#define LOG_WARN(msg, ...)  logger::log(log_level::WARN,  fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
#define LOG_INFO(msg, ...)  logger::log(log_level::INFO,  fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
#define LOG_DEBUG(msg, ...) logger::log(log_level::DEBUG, fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
#define LOG_TRACE(msg, ...) logger::log(log_level::TRACE, fmt::format(msg, ##__VA_ARGS__), __FILE__, __LINE__)
