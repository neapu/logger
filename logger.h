#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#if defined(ENABLE_FMT_LIB)
#include <fmt/format.h>
#endif
#if __cplusplus >= 202002L
#include <format>
#endif
#include <sstream>
#include <mutex>
#include <cstring>
#if __cplusplus >= 202002L
#include <source_location>
#endif

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

// #define NEAPU_LOG_LEVEL_NOLOG neapu::LogLevel::NOLOG
// #define NEAPU_LOG_LEVEL_ERROR neapu::LogLevel::ERROR
// #define NEAPU_LOG_LEVEL_WARNING neapu::LogLevel::WARNING
// #define NEAPU_LOG_LEVEL_INFO neapu::LogLevel::INFO
// #define NEAPU_LOG_LEVEL_DEBUG neapu::LogLevel::DEBUG

namespace neapu {
enum class LogLevel {
    NOLOG = 0,
    ERROR = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4
};
}

constexpr auto NEAPU_LOG_LEVEL_NOLOG = neapu::LogLevel::NOLOG;
constexpr auto NEAPU_LOG_LEVEL_ERROR = neapu::LogLevel::ERROR;
constexpr auto NEAPU_LOG_LEVEL_WARNING = neapu::LogLevel::WARNING;
constexpr auto NEAPU_LOG_LEVEL_INFO = neapu::LogLevel::INFO;
constexpr auto NEAPU_LOG_LEVEL_DEBUG = neapu::LogLevel::DEBUG;

#ifndef NEAPU_LOG_LEVEL
#define NEAPU_LOG_LEVEL NEAPU_LOG_LEVEL_DEBUG
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_ERROR
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
#define NEAPU_LOGE(...) neapu::Logger(neapu::LogLevel::ERROR, std::source_location::current()) << std::format(__VA_ARGS__)
#elif __cplusplus >= 202002L && defined(ENABLE_FMT_LIB)
#define NEAPU_LOGE(...) neapu::Logger(neapu::LogLevel::ERROR, std::source_location::current()) << fmt::format(__VA_ARGS__)
#elif defined(ENABLE_FMT_LIB)
#define NEAPU_LOGE(...) neapu::Logger(neapu::LogLevel::ERROR, __FILENAME__, __LINE__, __FUNCTION__) << fmt::format(__VA_ARGS__)
#else
#define NEAPU_LOGE(...)
#endif
#define NEAPU_LOGE_STREAM neapu::Logger(neapu::LogLevel::ERROR, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGE(...)
#define NEAPU_LOGE_STREAM if (false) neapu::Logger(neapu::LogLevel::ERROR, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_WARNING
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
#define NEAPU_LOGW(...) neapu::Logger(neapu::LogLevel::WARNING, std::source_location::current()) << std::format(__VA_ARGS__)
#elif __cplusplus >= 202002L && defined(ENABLE_FMT_LIB)
#define NEAPU_LOGW(...) neapu::Logger(neapu::LogLevel::WARNING, std::source_location::current()) << fmt::format(__VA_ARGS__)
#elif defined(ENABLE_FMT_LIB)
#define NEAPU_LOGW(...) neapu::Logger(neapu::LogLevel::WARNING, __FILENAME__, __LINE__, __FUNCTION__) << fmt::format(__VA_ARGS__)
#else
#define NEAPU_LOGW(...)
#endif
#define NEAPU_LOGW_STREAM neapu::Logger(neapu::LogLevel::WARNING, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGW(...)
#define NEAPU_LOGW_STREAM if (false) neapu::Logger(neapu::LogLevel::WARNING, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
#define NEAPU_LOGI(...) neapu::Logger(neapu::LogLevel::INFO, std::source_location::current()) << std::format(__VA_ARGS__)
#elif __cplusplus >= 202002L && defined(ENABLE_FMT_LIB)
#define NEAPU_LOGI(...) neapu::Logger(neapu::LogLevel::INFO, std::source_location::current()) << fmt::format(__VA_ARGS__)
#elif defined(ENABLE_FMT_LIB)
#define NEAPU_LOGI(...) neapu::Logger(neapu::LogLevel::INFO, __FILENAME__, __LINE__, __FUNCTION__) << fmt::format(__VA_ARGS__)
#else
#define NEAPU_LOGI(...)
#endif
#define NEAPU_LOGI_STREAM neapu::Logger(neapu::LogLevel::INFO, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGI(...)
#define NEAPU_LOGI_STREAM if (false) neapu::Logger(neapu::LogLevel::INFO, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_DEBUG
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
#define NEAPU_LOGD(...) neapu::Logger(neapu::LogLevel::DEBUG, std::source_location::current()) << std::format(__VA_ARGS__)
#elif __cplusplus >= 202002L && defined(ENABLE_FMT_LIB)
#define NEAPU_LOGD(...) neapu::Logger(neapu::LogLevel::DEBUG, std::source_location::current()) << fmt::format(__VA_ARGS__)
#elif defined(ENABLE_FMT_LIB)
#define NEAPU_LOGD(...) neapu::Logger(neapu::LogLevel::DEBUG, __FILENAME__, __LINE__, __FUNCTION__) << fmt::format(__VA_ARGS__)
#else
#define NEAPU_LOGD(...)
#endif
#define NEAPU_LOGD_STREAM neapu::Logger(neapu::LogLevel::DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGD(...)
#define NEAPU_LOGD_STREAM if (false) neapu::Logger(neapu::LogLevel::DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#define NEAPU_FUNC_TRACE neapu::FunctionTracer __tracer__(NEAPU_LOG_LEVEL_INFO, __FILENAME__, __FUNCTION__)
#else
#define NEAPU_FUNC_TRACE
#endif

#ifndef NEAPU_LOG_NO_SHORT_MACROS
#define LOGE(...) NEAPU_LOGE(__VA_ARGS__)
#define LOGW(...) NEAPU_LOGW(__VA_ARGS__)
#define LOGI(...) NEAPU_LOGI(__VA_ARGS__)
#define LOGD(...) NEAPU_LOGD(__VA_ARGS__)
#define LOGE_STREAM NEAPU_LOGE_STREAM
#define LOGW_STREAM NEAPU_LOGW_STREAM
#define LOGI_STREAM NEAPU_LOGI_STREAM
#define LOGD_STREAM NEAPU_LOGD_STREAM
#define FUNC_TRACE NEAPU_FUNC_TRACE
#endif

namespace neapu {
#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
class FunctionTracer final {
public:
    FunctionTracer(LogLevel level, const char* fileName, const char* funcName);
    ~FunctionTracer();

private:
    LogLevel m_level;
    const char* m_fileName;
    const char* m_funcName;
};
#endif

class Logger final {
public:
    static void setPrintLevel(LogLevel level);
    static void setLogLevel(LogLevel level, const std::string& logPath, const std::string& logPrefix = "");
    static void disableFuncName(const bool disable = true) { m_disableFuncName = disable; }

    Logger(LogLevel level, const char* fileName, int line, const char* funcName);
#if __cplusplus >= 202002L
    Logger(LogLevel level, const std::source_location& location = std::source_location::current());
#endif

    ~Logger();

    template <class T>
    Logger& operator<<(T&& t)
    {
        m_data << std::forward<T>(t);
        return *this;
    }

private:
    std::string makeLogString(LogLevel level);
    static std::string getTimeString();
    static void printLog(LogLevel level, const std::string& logText);
    static void writeLog(LogLevel level, const std::string& logText);
    static bool openFile();

private:
    LogLevel m_level;
    std::stringstream m_data;
    const char* m_fileName;
    int m_line;
    const char* m_funcName;

    static LogLevel m_printLevel;
    static LogLevel m_logLevel;
    static std::string m_logPath;
    static std::string m_logPrefix;
    static std::string m_logFileName;
    static FILE* m_pFile;
    static std::mutex m_fileMutex;
    static bool m_firstLog;
    static bool m_disableFuncName;
};
} // namespace neapu

#endif // LOGGER_H_