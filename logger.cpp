#include "logger.h"
#include <chrono>

namespace {
const char* LogLevelText[] = { "NOLOG", "ERROR", "WARNING", "INFO", "DEBUG"};
}

namespace neapu {
int Logger::m_printLevel = NEAPU_LOG_LEVEL;
std::function<void(int, const std::string&)> Logger::m_printHandler = nullptr;

#if !defined(NEAPU_LOG_DISABLE_FUNC_TRACE) && NEAPU_LOG_LEVEL >= NEAPU_LOG_LEVEL_INFO
FunctionTracer::FunctionTracer(int level, const char* fileName, const char* funcName)
    : m_level(level),
    m_fileName(fileName),
    m_funcName(funcName)
{
    Logger(level, nullptr, 0, nullptr).format("[FuncTrace]Enter function: [{}][{}]", fileName, funcName);
}

FunctionTracer::~FunctionTracer()
{
    Logger(m_level, nullptr, 0, nullptr).format("[FuncTrace]Leave function: [{}][{}]", m_fileName, m_funcName);
}
#endif

void Logger::setPrintLevel(const int level)
{
    m_printLevel = level;
}

Logger::Logger(int level, const char* fileName, int line, const char* funcName)
    : m_level(level)
    , m_fileName(fileName)
    , m_line(line)
    , m_funcName(funcName)
{
}

void Logger::setPrintHandler(std::function<void(int, const std::string &)> handler)
{
    m_printHandler = handler;
}

Logger::~Logger()
{
    if (m_level <= m_printLevel) {
        const std::string logText = makeLogString(m_level);
        printLog(m_level, logText);
    }
}

std::string Logger::makeLogString(int level)
{
    std::string logText;
    if (!m_fileName || !m_line || !m_funcName) {
        logText = std::format("[{}]", getTimeString());
    } else {
        if (level < 0 || level >= std::size(LogLevelText)) {
            level = 0;
        }
        std::string logLevel = LogLevelText[level];
        logText = std::format("[{}][{}][{}:{}][{}]", getTimeString(), logLevel, m_fileName, m_line, m_funcName);
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
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                       tm.tm_hour, tm.tm_min, tm.tm_sec, now_ms.count());
}

void Logger::printLog(int level, const std::string& logText)
{
    if (level > m_printLevel) return;
    if (m_printHandler) {
        m_printHandler(level, logText);
    }
}
}