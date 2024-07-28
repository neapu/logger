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

namespace neapu {
LogLevel Logger::m_printLevel = LogLevel::LM_DEBUG;
LogLevel Logger::m_logLevel = LogLevel::LM_NOLOG;
std::string Logger::m_logPath;
std::string Logger::m_logPrefix;
std::string Logger::m_logFileName;
FILE* Logger::m_pFile = nullptr;
std::mutex Logger::m_fileMutex;
bool Logger::m_firstLog = true;

void Logger::setPrintLevel(const LogLevel level)
{
    m_printLevel = level;
}

void Logger::setLogLevel(LogLevel level, const std::string& logPath, const std::string& logPrefix)
{
    if (logPath.empty()) {
        m_logLevel = LogLevel::LM_NOLOG;
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
            m_logLevel = LogLevel::LM_NOLOG;
            return;
        }
    }
    m_logPrefix = logPrefix;
}

Logger::Logger(const LogLevel level, const std::source_location& loc)
    : m_level(level)
    , m_loc(loc)
{
}

Logger::~Logger()
{
    if (m_level <= m_printLevel || m_level <= m_logLevel) {
        const std::string logText = makeLogString(m_level, m_loc, m_data.str());
        printLog(m_level, logText);
        writeLog(m_level, logText);
    }
}

std::string Logger::makeLogString(const LogLevel level, const std::source_location& loc, const std::string& logText)
{
    std::string threadId;
#ifdef _WIN32
    threadId = std::to_string(GetCurrentThreadId());
#else
    pthread_t tid = pthread_self();
    threadId = std::to_string((uint64_t)tid);
#endif
    std::string logLevel;
    switch (level) {
    case LogLevel::LM_DEADLY: logLevel = "Deadly";
        break;
    case LogLevel::LM_ERROR: logLevel = "Error";
        break;
    case LogLevel::LM_WARNING: logLevel = "Warning";
        break;
    case LogLevel::LM_INFO: logLevel = "Info";
        break;
    case LogLevel::LM_DEBUG: logLevel = "Debug";
        break;
    case LogLevel::LM_VERBOSE: logLevel = "Verbose";
        break;
    default: logLevel = "Unknown";
        break;
    }
    return std::format("[{}][{}][{}][{}:{}][{}]: {}", getTimeString(), logLevel, threadId, loc.file_name(), loc.line(), loc.function_name(),
                       logText);
}

std::string Logger::getTimeString()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto now_c = system_clock::to_time_t(now);
    const auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    auto [tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst] = *std::localtime(&now_c);
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", tm_year + 1900, tm_mon + 1, tm_mday,
                       tm_hour, tm_min, tm_sec, now_ms.count());
}

void Logger::printLog(const LogLevel level, const std::string& logText)
{
    if (level > m_printLevel) return;
    std::cout << logText << std::endl;
#ifdef __ANDROID__
    switch (m_level) {
    case LM_DEADLY:
        __android_log_print(ANDROID_LOG_FATAL, "neapu_log", "%s\n", logText.c_str());
        break;
    case LM_ERROR:
        __android_log_print(ANDROID_LOG_ERROR, "neapu_log", "%s\n", logText.c_str());
        break;
    case LM_WARNING:
        __android_log_print(ANDROID_LOG_WARN, "neapu_log", "%s\n", logText.c_str());
        break;
    case LM_INFO:
        __android_log_print(ANDROID_LOG_INFO, "neapu_log", "%s\n", logText.c_str());
        break;
    case LM_DEBUG:
        __android_log_print(ANDROID_LOG_DEBUG, "neapu_log", "%s\n", logText.c_str());
        break;
    case LM_VERBOSE:
        __android_log_print(ANDROID_LOG_VERBOSE, "neapu_log", "%s\n", logText.c_str());
        break;
    }
#endif
}

void Logger::writeLog(const LogLevel level, const std::string& logText)
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