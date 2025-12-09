#include "logger.h"
#include <chrono>
#include <iostream>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <Windows.h>
#define access _access
#define mkdir(x, y) _mkdir(x)
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#endif
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace {
const char* LogLevelText[] = { "NOLOG", "ERROR", "WARNING", "INFO", "DEBUG"};
}

namespace neapu {
LogLevel Logger::m_printLevel = static_cast<LogLevel>(NEAPU_LOG_LEVEL);
LogLevel Logger::m_logLevel = NEAPU_LOG_LEVEL_NOLOG;
std::string Logger::m_logPath;
std::string Logger::m_logPrefix;
std::string Logger::m_logFileName;
FILE* Logger::m_pFile = nullptr;
std::mutex Logger::m_fileMutex;
bool Logger::m_firstLog = true;

#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
FunctionTracer::FunctionTracer(LogLevel level, const char* fileName, const char* funcName)
    : m_level(level),
    m_fileName(fileName),
    m_funcName(funcName)
{
    // Logger(level, nullptr, 0, nullptr).format("[FuncTrace]Enter function: [{}][{}]", fileName, funcName);
    Logger(level, fileName, 0, funcName) << "[FuncTrace]Enter function: [" << fileName << "][" << funcName << "]";
}

FunctionTracer::~FunctionTracer()
{
    // Logger(m_level, nullptr, 0, nullptr).format("[FuncTrace]Leave function: [{}][{}]", m_fileName, m_funcName);
    Logger(m_level, m_fileName, 0, m_funcName) << "[FuncTrace]Leave function: [" << m_fileName << "][" << m_funcName << "]";
}
#endif

void Logger::setPrintLevel(const LogLevel level)
{
    m_printLevel = level;
}

void Logger::setLogLevel(LogLevel level, const std::string& logPath, const std::string& logPrefix)
{
    if (logPath.empty()) {
        m_logLevel = NEAPU_LOG_LEVEL_NOLOG;
        return;
    }
    m_logPath = logPath;
    if (m_logPath.back() != '/' && m_logPath.back() != '\\') {
        m_logPath += '/';
    }
    m_logLevel = level;
    if (access(m_logPath.c_str(), 0) == -1) {
        if (mkdir(m_logPath.c_str(), 0744) == -1) {
            std::cerr << "Failed to create log directory: " << m_logPath << std::endl;
            m_logLevel = NEAPU_LOG_LEVEL_NOLOG;
            return;
        }
    }
    m_logPrefix = logPrefix;
}

Logger::Logger(LogLevel level, const char* fileName, int line, const char* funcName)
    : m_level(level)
    , m_fileName(fileName)
    , m_line(line)
    , m_funcName(funcName)
{
}

#if __cplusplus >= 202002L
Logger::Logger(LogLevel level, const std::source_location& location)
    : m_level(level)
#ifdef _WIN32
    , m_fileName(strrchr(location.file_name(), '\\') ? strrchr(location.file_name(), '\\') + 1 : location.file_name())
#else
    , m_fileName(strrchr(location.file_name(), '/') ? strrchr(location.file_name(), '/') + 1 : location.file_name())
#endif
    , m_line(static_cast<int>(location.line()))
    , m_funcName(location.function_name())
{
}
#endif

Logger::~Logger()
{
    if (m_level <= m_printLevel || m_level <= m_logLevel) {
        const std::string logText = makeLogString(m_level);
        printLog(m_level, logText);
        writeLog(m_level, logText);
    }
}

std::string Logger::makeLogString(LogLevel level)
{
    std::string threadId;
#ifdef _WIN32
    threadId = std::to_string(GetCurrentThreadId());
#elif defined(__APPLE__)
    threadId = std::to_string(pthread_mach_thread_np(pthread_self()));
#else
    const pthread_t tid = pthread_self();
    threadId = std::to_string(tid);
#endif
    std::string logText;
    if (!m_fileName || !m_line || !m_funcName) {
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
        logText = std::format("[{}][{}]", getTimeString(), threadId);
#elif defined(ENABLE_FMT_LIB)
        logText = fmt::format("[{}][{}]", getTimeString(), threadId);
#else
        std::stringstream ss;
        ss << "[" << getTimeString() << "][" << threadId << "]";
        logText = ss.str();
#endif
    } else {
        std::string logLevel = LogLevelText[static_cast<int>(level)];
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
        logText = std::format("[{}][{}][{}][{}:{}][{}]", getTimeString(), threadId, logLevel, m_fileName, m_line, m_funcName);
#elif defined(ENABLE_FMT_LIB)
        logText = fmt::format("[{}][{}][{}][{}:{}][{}]", getTimeString(), threadId, logLevel, m_fileName, m_line, m_funcName);
#else
        std::stringstream ss;
        ss << "[" << getTimeString() << "][" << threadId << "][" << logLevel << "][" << m_fileName << ":" << m_line << "][" << m_funcName << "]";
        logText = ss.str();
#endif
    }

    logText += m_data.str();
    return logText;
}

std::string Logger::getTimeString()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto now_c = system_clock::to_time_t(now);
    const auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm = *std::localtime(&now_c);
#if __cplusplus >= 202002L && !defined(ENABLE_FMT_LIB)
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                       tm.tm_hour, tm.tm_min, tm.tm_sec, now_ms.count());
#elif defined(ENABLE_FMT_LIB)
    return fmt::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                       tm.tm_hour, tm.tm_min, tm.tm_sec, now_ms.count());
#else
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                       tm.tm_hour, tm.tm_min, tm.tm_sec, (int)now_ms.count());
    return std::string(buffer);
#endif
}

void Logger::printLog(LogLevel level, const std::string& logText)
{
    if (level > m_printLevel) return;
    std::cout << logText << std::endl;
#ifdef __ANDROID__
    switch (m_level) {
    case NEAPU_LOG_LEVEL_ERROR:
        __android_log_print(ANDROID_LOG_ERROR, "neapu_log", "%s\n", logText.c_str());
        break;
    case NEAPU_LOG_LEVEL_WARNING:
        __android_log_print(ANDROID_LOG_WARN, "neapu_log", "%s\n", logText.c_str());
        break;
    case NEAPU_LOG_LEVEL_INFO:
        __android_log_print(ANDROID_LOG_INFO, "neapu_log", "%s\n", logText.c_str());
        break;
    case NEAPU_LOG_LEVEL_DEBUG:
        __android_log_print(ANDROID_LOG_DEBUG, "neapu_log", "%s\n", logText.c_str());
        break;
    }
#endif
}

void Logger::writeLog(LogLevel level, const std::string& logText)
{
    if (level > m_logLevel) return;
    if (m_logPath.empty()) return;

    if (!openFile()) {
        std::cerr << "Failed to open log file: " << m_logPath << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_firstLog) {
        m_firstLog = false;
        fprintf(m_pFile, "------------------------%s------------------------\n", getTimeString().c_str());
    }
    fprintf(m_pFile, "%s\n", logText.c_str());
    fflush(m_pFile);
}

bool Logger::openFile()
{
    std::time_t now_c = time(nullptr);
    char temp[64];
    strftime(temp, 64, "%Y%m%d", localtime(&now_c));
    char szNewFile[128];
    snprintf(szNewFile, 128, "%s%s_%s.log", m_logPath.c_str(), m_logPrefix.c_str(), temp);
    if (m_pFile) {
        if (m_logFileName == szNewFile) return true;
        fclose(m_pFile);
    }
    m_logFileName = szNewFile;
    m_pFile = fopen(szNewFile, "a");
    if (!m_pFile) return false;
    return true;
}
}