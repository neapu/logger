#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <format>
#include <sstream>
#include <mutex>
#include <cstring>

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

#define NEAPU_LOG_LEVEL_NOLOG 0
#define NEAPU_LOG_LEVEL_ERROR 1
#define NEAPU_LOG_LEVEL_WARNING 2
#define NEAPU_LOG_LEVEL_INFO 3
#define NEAPU_LOG_LEVEL_DEBUG 4

#ifndef NEAPU_LOG_LEVEL
#define NEAPU_LOG_LEVEL NEAPU_LOG_LEVEL_DEBUG
#endif


#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_ERROR
#define NEAPU_LOGE(...) neapu::Logger(NEAPU_LOG_LEVEL_ERROR, __FILENAME__, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGE_STREAM neapu::Logger(NEAPU_LOG_LEVEL_ERROR, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGE(...)
#define NEAPU_LOGE_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_WARNING
#define NEAPU_LOGW(...) neapu::Logger(NEAPU_LOG_LEVEL_WARNING, __FILENAME__, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGW_STREAM neapu::Logger(NEAPU_LOG_LEVEL_WARNING, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGW(...)
#define NEAPU_LOGW_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#define NEAPU_LOGI(...) neapu::Logger(NEAPU_LOG_LEVEL_INFO, __FILENAME__, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGI_STREAM neapu::Logger(NEAPU_LOG_LEVEL_INFO, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGI(...)
#define NEAPU_LOGI_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_DEBUG
#define NEAPU_LOGD(...) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGD_STREAM neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGD(...)
#define NEAPU_LOGD_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, __FUNCTION__)
#endif

#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#define NEAPU_FUNC_TRACE neapu::FunctionTracer __tracer__(NEAPU_LOG_LEVEL_INFO, __FILENAME__, __FUNCTION__)
#else
#define NEAPU_FUNC_TRACE
#endif
namespace neapu {
#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
class FunctionTracer final {
public:
    FunctionTracer(int level, const char* fileName, const char* funcName);
    ~FunctionTracer();

private:
    int m_level;
    const char* m_fileName;
    const char* m_funcName;
};
#endif

class Logger final {
public:
    static void setPrintLevel(int level);
    static void setLogLevel(int level, const std::string& logPath, const std::string& logPrefix = "");
    static void disableFuncName(const bool disable = true) { m_disableFuncName = disable; }

    Logger(int level, const char* fileName, int line, const char* funcName);
    ~Logger();

    template <class T>
    Logger& operator<<(T&& t)
    {
        m_data << std::forward<T>(t);
        return *this;
    }

    template <class... Args>
    Logger& format(std::format_string<Args...> fmt, Args&&... args)
    {
        m_data << std::format(fmt, std::forward<Args>(args)...);
        return *this;
    }

private:
    std::string makeLogString(int level);
    static std::string getTimeString();
    static void printLog(int level, const std::string& logText);
    static void writeLog(int level, const std::string& logText);
    static bool openFile();

private:
    int m_level;
    std::stringstream m_data;
    const char* m_fileName;
    int m_line;
    const char* m_funcName;
    static int m_printLevel;
    static int m_logLevel;
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