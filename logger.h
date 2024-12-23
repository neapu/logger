#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <format>
#include <sstream>
#include <cstring>
#include <functional>

#ifndef NEAPU_FILENAME
#define NEAPU_FILENAME (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
// #define NEAPU_FILENAME __FILE__
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
#define NEAPU_LOGE(...) neapu::Logger(NEAPU_LOG_LEVEL_ERROR, NEAPU_FILENAME, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGE_STREAM neapu::Logger(NEAPU_LOG_LEVEL_ERROR, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGE(...)
#define NEAPU_LOGE_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_WARNING
#define NEAPU_LOGW(...) neapu::Logger(NEAPU_LOG_LEVEL_WARNING, NEAPU_FILENAME, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGW_STREAM neapu::Logger(NEAPU_LOG_LEVEL_WARNING, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGW(...)
#define NEAPU_LOGW_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#define NEAPU_LOGI(...) neapu::Logger(NEAPU_LOG_LEVEL_INFO, NEAPU_FILENAME, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGI_STREAM neapu::Logger(NEAPU_LOG_LEVEL_INFO, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGI(...)
#define NEAPU_LOGI_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#endif

#if NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_DEBUG
#define NEAPU_LOGD(...) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__).format(__VA_ARGS__)
#define NEAPU_LOGD_STREAM neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#else
#define NEAPU_LOGD(...)
#define NEAPU_LOGD_STREAM if (false) neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, NEAPU_FILENAME, __LINE__, __FUNCTION__)
#endif

#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
#define NEAPU_FUNC_TRACE neapu::FunctionTracer __tracer__(NEAPU_LOG_LEVEL_INFO, NEAPU_FILENAME, __FUNCTION__)
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
    static void setPrintHandler(std::function<void(int, const std::string&)> handler);

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
private:
    int m_level;
    std::stringstream m_data;
    const char* m_fileName;
    int m_line;
    const char* m_funcName;
    static int m_printLevel;
    static std::function<void(int, const std::string&)> m_printHandler;
};
} // namespace neapu

#endif // LOGGER_H_