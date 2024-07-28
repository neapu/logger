#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>
#include <format>
#include <source_location>
#include <sstream>
#include <mutex>

#ifdef ENABLE_OLD_STYLE_LOG
#define LOG_DEBUG neapu::Logger(neapu::LogLevel::LM_DEBUG, std::source_location::current())
#define LOG_INFO neapu::Logger(neapu::LogLevel::LM_INFO, std::source_location::current())
#define LOG_WARNING neapu::Logger(neapu::LogLevel::LM_WARNING, std::source_location::current())
#define LOG_ERROR neapu::Logger(neapu::LogLevel::LM_ERROR, std::source_location::current())
#define LOG_DEADLY neapu::Logger(neapu::LogLevel::LM_DEADLY, std::source_location::current())
#define LOG_VERBOSE neapu::Logger(neapu::LogLevel::LM_VERBOSE, std::source_location::current())
#endif

#define FUNC_TRACE neapu::FunctionTracer __tracer__

namespace neapu {
enum LogLevel {
    LM_NOLOG = 0,
    LM_DEADLY = 1,
    LM_ERROR = 2,
    LM_WARNING = 3,
    LM_INFO = 4,
    LM_DEBUG = 5,
    LM_VERBOSE = 6
};

class FunctionTracer final {
public:
    FunctionTracer(LogLevel level = LM_DEBUG, const std::source_location& loc = std::source_location::current());
    ~FunctionTracer();

private:
    LogLevel m_level;
    std::source_location m_loc;
};

class Logger final {
public:
    static void setPrintLevel(LogLevel level);
    static void setLogLevel(LogLevel level, const std::string& logPath, const std::string& logPrefix = "");

    explicit Logger(LogLevel level, const std::source_location& loc = std::source_location::current());
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

    template <class... Args>
    static Logger& deadly(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_DEADLY, loc).format(fmt, std::forward<Args>(args)...);
    }
#define deadly(...) deadly(std::source_location::current() , __VA_ARGS__)

    template <class... Args>
    static Logger& error(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_ERROR, loc).format(fmt, std::forward<Args>(args)...);
    }
#define error(...) error(std::source_location::current() , __VA_ARGS__)

    template <class... Args>
    static Logger& warning(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_WARNING, loc).format(fmt, std::forward<Args>(args)...);
    }
#define warning(...) warning(std::source_location::current() , __VA_ARGS__)

    template <class... Args>
    static Logger& info(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_INFO, loc).format(fmt, std::forward<Args>(args)...);
    }
#define info(...) info(std::source_location::current() , __VA_ARGS__)

    template <class... Args>
    static Logger& debug(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_DEBUG, loc).format(fmt, std::forward<Args>(args)...);
    }
#define debug(...) debug(std::source_location::current() , __VA_ARGS__)

    template <class... Args>
    static Logger& verbose(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        return Logger(LogLevel::LM_VERBOSE, loc).format(fmt, std::forward<Args>(args)...);
    }
#define verbose(...) verbose(std::source_location::current() , __VA_ARGS__)

private:
    static std::string makeLogString(LogLevel level, const std::source_location& loc, const std::string& logText);
    static std::string getTimeString();
    static void printLog(LogLevel level, const std::string& logText);
    static void writeLog(LogLevel level, const std::string& logText);
    static bool openFile();

private:
    LogLevel m_level;
    std::stringstream m_data;
    std::source_location m_loc;
    static LogLevel m_printLevel;
    static LogLevel m_logLevel;
    static std::string m_logPath;
    static std::string m_logPrefix;
    static std::string m_logFileName;
    static FILE* m_pFile;
    static std::mutex m_fileMutex;
    static bool m_firstLog;
};
} // namespace neapu

#endif // LOGGER_H_