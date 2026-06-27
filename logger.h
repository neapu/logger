#pragma once

#include <sstream>
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <map>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <ctime>
#ifdef _WIN32
#include <stringapiset.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <thread>

#include <fmt/format.h>

namespace logger {
enum class LogLevel {
    NONE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
};
std::ostream& operator<<(std::ostream& os, LogLevel level);

class Logger {
public:
    Logger(LogLevel level, const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);
    ~Logger();

    Logger& operator<<(const char* s);
    Logger& operator<<(std::string_view sv);
    Logger& operator<<(const std::string& s);
#ifdef _WIN32
    Logger& operator<<(const wchar_t* ws);
    Logger& operator<<(std::wstring_view wsv);
    Logger& operator<<(const std::wstring& ws);

    template <class T>
        requires (!std::is_same_v<std::remove_cvref_t<T>, std::wstring>
               && !std::is_same_v<std::remove_cvref_t<T>, std::wstring_view>
               && !std::is_same_v<std::remove_cvref_t<T>, const wchar_t*>
               && !std::is_same_v<std::remove_cvref_t<T>, wchar_t*>)
    Logger& operator<<(T&& t)
    {
        m_data << std::forward<T>(t);
        return *this;
    }
#endif

    template<typename... Args>
    Logger& format(fmt::format_string<Args...> fmt, Args&&... args)
    {
        auto formatted = fmt::format(fmt, std::forward<Args>(args)...);
        m_data << std::string_view(formatted);
        return *this;
    }
#ifdef _WIN32
    template<typename... Args>
    Logger& format_w(fmt::wformat_string<Args...> fmt, Args&&... args)
    {
        auto formatted = fmt::format(fmt, std::forward<Args>(args)...);
        const wchar_t* data = formatted.c_str();
        int len = static_cast<int>(formatted.size());
        int needed = ::WideCharToMultiByte(CP_UTF8, 0, data, len, nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            std::string out(static_cast<size_t>(needed), '\0');
            int written = ::WideCharToMultiByte(CP_UTF8, 0, data, len, out.data(), needed, nullptr, nullptr);
            if (written > 0) {
                m_data.write(out.data(), static_cast<std::streamsize>(out.size()));
            }
        }
        return *this;
    }
#endif

    static void setLogPath(const std::string& path);
    static void openNewFile(const std::string& channel);
    static void setMaxFileSize(std::streamoff bytes);
    static void rotateIfNeeded(const std::string& channel);
    static void setPrintLevel(LogLevel level) { s_printLevel = level; }
    static void setLogLevel(LogLevel level) { s_logLevel = level; }
    static void pureLog(LogLevel level, const std::string& channel, const std::string& message);
    static void setLockingEnabled(bool enabled);
private:
    std::stringstream m_data;
    LogLevel m_level;
    std::string m_file;
    std::string m_function;
    int m_line;
    std::string m_channel;
    static std::string s_logPath;
    static std::map<std::string, std::ofstream> s_ofstreamMap;
    static std::streamoff s_maxFileSize;
    static LogLevel s_printLevel;
    static LogLevel s_logLevel;
    static std::recursive_mutex s_mutex;
    static std::atomic<bool> s_lockEnabled;
};
Logger LogDebug(const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);
Logger LogInfo(const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);
Logger LogWarn(const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);
Logger LogError(const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);

class FunctionTracer {
public:
    explicit FunctionTracer(LogLevel level = LogLevel::INFO, const std::string& channel = std::string{}, const std::string& file = __FILE__, const std::string& function = __FUNCTION__, int line = __LINE__);
    ~FunctionTracer();
private:
    LogLevel m_level;
    std::string m_channel;
    std::string m_file;
    std::string m_function;
    int m_line;
    std::string m_logPrefix;
};
}