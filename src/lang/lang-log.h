//
// Created by Thinkpad on 2017/9/8.
//

#ifndef AICAST_BACKHAUL_LOG_H
#define AICAST_BACKHAUL_LOG_H

#include <iostream>
#include <spdlog.h>
#include "spdlog/fmt/bundled/ostream.h"

using cstr = const char * const;

static constexpr cstr past_last_slash(cstr str, cstr last_slash)
{
    return
            *str == '\0' ? last_slash :
            *str == '/'  ? past_last_slash(str + 1, str + 1) :
            past_last_slash(str + 1, last_slash);
}

static constexpr cstr past_last_slash(cstr str)
{
    return past_last_slash(str, str);
}

#define __SHORT_FILE__ ({constexpr cstr sf__ {past_last_slash(__FILE__)}; sf__;})

#define PRINTF(fmt, ...) Log::print(fmt, ##__VA_ARGS__)
#define DEBUGF(fmt, ...) Log::debug("DBG {}:{}:{} " fmt, __SHORT_FILE__ , __LINE__, __func__, ##__VA_ARGS__)
#define INFOF(fmt, ...)  Log::info("INF {}:{}:{} " fmt, __SHORT_FILE__ , __LINE__, __func__, ##__VA_ARGS__)
#define TEST(fmt, ...)  Log::info("TEST {}:{}:{} " fmt, __SHORT_FILE__ , __LINE__, __func__, ##__VA_ARGS__)
#define ERRORF(fmt, ...) Log::error("ERR {}:{}:{} " fmt, __SHORT_FILE__ , __LINE__, __func__, ##__VA_ARGS__)
#define WARNF(fmt, ...)  Log::warn("WRN {}:{}:{} " fmt, __SHORT_FILE__ , __LINE__, __func__, ##__VA_ARGS__)
#define FATALF(fmt, ...) Log::fatal("FATAL {}:{}:{} {} " fmt, __SHORT_FILE__ , __LINE__, __func__, std::strerror(errno), ##__VA_ARGS__)
#define ERRNOF(fmt, ...) Log::error("ERR {}:{}:{} ({}) " fmt, __SHORT_FILE__ , __LINE__, __func__, std::strerror(errno), ##__VA_ARGS__)

class Log {
public:
    typedef std::map<std::string, spdlog::level::level_enum> Levels;
    typedef std::shared_ptr<spdlog::logger> Logger;
    static Levels _levels;
    static Logger _logger;

    static std::string get_level() {
        switch (_logger->level()) {
            case spdlog::level::debug:
                return "debug";
            case spdlog::level::info:
                return "info";
            case spdlog::level::warn:
                return "warn";
            case spdlog::level::err:
                return "error";
            case spdlog::level::critical:
                return "fatal";
            default:
                return "";
        }
    }
    static bool set_level(const std::string &level) {
        bool valid = true;
        if (level == "debug") {
            _logger->set_level(spdlog::level::debug);
        } else if (level == "info") {
            _logger->set_level(spdlog::level::info);
        } else if (level == "warn") {
            _logger->set_level(spdlog::level::warn);
        } else if (level == "error") {
            _logger->set_level(spdlog::level::err);
        } else if (level == "fatal") {
            _logger->set_level(spdlog::level::critical);
        } else {
            valid = false;
            ERRORF("unknown log level {}", level);
        }
        if (valid) {
            PRINTF("set log level {}", level);
        }
        return valid;
    }

    template <typename Arg1, typename... Args>
    static void print(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(_logger->level(), fmt, arg1, args...);
    };
    template <typename Arg1, typename... Args>
    static void debug(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(spdlog::level::debug, fmt, arg1, args...);
    };
    template <typename Arg1, typename... Args>
    static void info(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(spdlog::level::info, fmt, arg1, args...);
    };
    template <typename Arg1, typename... Args>
    static void warn(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(spdlog::level::warn, fmt, arg1, args...);
    };
    template <typename Arg1, typename... Args>
    static void error(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(spdlog::level::err, fmt, arg1, args...);
    };
    template <typename Arg1, typename... Args>
    static void fatal(const char* fmt, const Arg1& arg1, const Args&... args) {
        _logger->log(spdlog::level::critical, fmt, arg1, args...);
        exit(-1);
    };
};



#endif //AICAST_BACKHAUL_LOG_H
