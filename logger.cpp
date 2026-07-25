#include "logger.h"

namespace {
unsigned long getProcessId()
{
#ifdef _WIN32
    return static_cast<unsigned long>(_getpid());
#else
    return static_cast<unsigned long>(getpid());
#endif
}
std::string getTimeString()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto sec_tp = floor<seconds>(now);
    const auto ms = duration_cast<milliseconds>(now - sec_tp).count();
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[32] = {};
    std::strftime(buf, sizeof(buf), "%F %T", &tm);
    return std::format("{}.{:03}", std::string(buf), ms);
}

std::string getTimeOnlyString()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto sec_tp = floor<seconds>(now);
    const auto ms = duration_cast<milliseconds>(now - sec_tp).count();
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[32] = {};
    std::strftime(buf, sizeof(buf), "%T", &tm);
    return std::format("{}.{:03}", std::string(buf), ms);
}

std::string levelToString(logger::LogLevel level)
{
    using enum logger::LogLevel;
    switch (level) {
    case ERROR: return "ERROR";
    case WARN: return "WARN";
    case INFO: return "INFO";
    case DEBUG: return "DEBUG";
    default: return std::string{};
    }
}
}

namespace logger {
std::ostream& operator<<(std::ostream& os, LogLevel level)
{
    os << levelToString(level);
    return os;
}

std::string Logger::s_logPath{};
std::map<std::string, std::ofstream> Logger::s_ofstreamMap{};
std::streamoff Logger::s_maxFileSize = 1024 * 1024;
LogLevel Logger::s_printLevel = LogLevel::DEBUG;
LogLevel Logger::s_logLevel = LogLevel::NONE;
std::recursive_mutex Logger::s_mutex{};
std::atomic<bool> Logger::s_lockEnabled{true};
bool Logger::s_consoleConciseEnabled{false};
Logger::Logger(LogLevel level, const std::string& channel, const std::source_location& location)
    : m_level(level), m_location(location), m_channel(channel)
{
    if (m_channel.empty()) {
        m_channel = "Default";
    }
}
Logger::~Logger()
{
    if (m_data.str().empty() || m_level == LogLevel::NONE) {
        return;
    }
    if (m_level > s_printLevel && (s_logPath.empty() || m_level > s_logLevel)) {
        return;
    }

    auto threadId = std::this_thread::get_id();
    const auto fileName = std::filesystem::path(m_location.file_name()).filename().string();

    std::stringstream fileSs;
    fileSs << "[" << getTimeString() << "]";
    fileSs << "[" << m_channel << "]";
    fileSs << "[" << levelToString(m_level) << "]";
    fileSs << "[TID:" << threadId << "]";
    fileSs << "[" << fileName << ":" << m_location.line() << "]";
    const std::string fileMsg = fileSs.str() + m_data.str();

    std::string consoleMsg;
    if (s_consoleConciseEnabled) {
        consoleMsg = "[" + getTimeOnlyString() + "]" + m_data.str();
    } else {
        consoleMsg = fileMsg;
    }

    output(m_level, m_channel, consoleMsg, fileMsg);
}

Logger& Logger::operator<<(const char* s)
{
    if (s) {
        m_data.write(s, static_cast<std::streamsize>(std::char_traits<char>::length(s)));
    } else {
        m_data << "(null)";
    }
    return *this;
}

Logger& Logger::operator<<(std::string_view sv)
{
    m_data.write(sv.data(), static_cast<std::streamsize>(sv.size()));
    return *this;
}

Logger& Logger::operator<<(const std::string& s)
{
    m_data.write(s.data(), static_cast<std::streamsize>(s.size()));
    return *this;
}
#ifdef _WIN32
Logger& Logger::operator<<(const wchar_t* ws)
{
    if (ws) {
        std::wstring_view wsv(ws);
        int needed = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            std::string out(static_cast<size_t>(needed), '\0');
            int written = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), out.data(), needed, nullptr, nullptr);
            if (written > 0) {
                m_data.write(out.data(), static_cast<std::streamsize>(out.size()));
            }
        }
    } else {
        m_data << "(null)";
    }
    return *this;
}
Logger& Logger::operator<<(std::wstring_view wsv)
{
    int needed = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), nullptr, 0, nullptr, nullptr);
    if (needed > 0) {
        std::string out(static_cast<size_t>(needed), '\0');
        int written = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), out.data(), needed, nullptr, nullptr);
        if (written > 0) {
            m_data.write(out.data(), static_cast<std::streamsize>(out.size()));
        }
    }
    return *this;
}
Logger& Logger::operator<<(const std::wstring& ws)
{
    std::wstring_view wsv(ws);
    int needed = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), nullptr, 0, nullptr, nullptr);
    if (needed > 0) {
        std::string out(static_cast<size_t>(needed), '\0');
        int written = ::WideCharToMultiByte(CP_UTF8, 0, wsv.data(), static_cast<int>(wsv.size()), out.data(), needed, nullptr, nullptr);
        if (written > 0) {
            m_data.write(out.data(), static_cast<std::streamsize>(out.size()));
        }
    }
    return *this;
}
#endif
void Logger::setLogPath(const std::string& path)
{
    s_logPath = path;
    if (s_logPath.ends_with('/') || s_logPath.ends_with('\\')) {
        s_logPath.pop_back();
    }
    std::error_code ec;
    std::filesystem::create_directories(s_logPath, ec);
}
void Logger::openNewFile(const std::string& channel)
{
    std::unique_lock<std::recursive_mutex> lock;
    if (s_lockEnabled.load()) {
        lock = std::unique_lock<std::recursive_mutex>(s_mutex);
    }
    std::ofstream os;
    os.open(s_logPath + "/" + channel + ".log", std::ios::out | std::ios::app);
    if (!os.is_open()) {
        return;
    }
    os << "[" << getTimeString() << "][HEADER][PID:" << getProcessId() << "]" << std::endl;
    s_ofstreamMap.emplace(channel, std::move(os));
}
void Logger::setMaxFileSize(std::streamoff bytes)
{
    s_maxFileSize = bytes;
}
void Logger::rotateIfNeeded(const std::string& channel)
{
    std::unique_lock<std::recursive_mutex> lock;
    if (s_lockEnabled.load()) {
        lock = std::unique_lock<std::recursive_mutex>(s_mutex);
    }
    auto it = s_ofstreamMap.find(channel);
    if (it == s_ofstreamMap.end()) {
        return;
    }
    if (it->second.tellp() <= s_maxFileSize) {
        return;
    }
    it->second.close();
    s_ofstreamMap.erase(it);
    using namespace std::chrono;
    const auto now = system_clock::now();
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tm);
    const std::string timestamp = buf;
    const std::string baseName = s_logPath + "/" + channel + ".log";
    const std::string oldDir = s_logPath + "/old";
    std::error_code ec;
    std::filesystem::create_directories(oldDir, ec);
    const std::string newName = oldDir + "/" + channel + "_" + timestamp + ".log";
    std::filesystem::rename(baseName, newName, ec);
    openNewFile(channel);
}
void Logger::pureLog(LogLevel level, const std::string& channel, const std::string& message)
{
    output(level, channel, message, message);
}
void Logger::output(LogLevel level, const std::string& channel, const std::string& consoleMsg, const std::string& fileMsg)
{
    // print to console
    if (level <= s_printLevel && s_printLevel != LogLevel::NONE) {
        std::cout << consoleMsg << std::endl;
    }

    // log to file
    if (s_logPath.empty() || level > s_logLevel || s_logLevel == LogLevel::NONE) {
        return;
    }

    std::unique_lock<std::recursive_mutex> lock;
    if (s_lockEnabled.load()) {
        lock = std::unique_lock<std::recursive_mutex>(s_mutex);
    }

    auto it = s_ofstreamMap.find(channel);
    if (it == s_ofstreamMap.end()) {
        openNewFile(channel);
        it = s_ofstreamMap.find(channel);
        if (it == s_ofstreamMap.end()) {
            return;
        }
    }

    rotateIfNeeded(channel);
    it = s_ofstreamMap.find(channel);
    if (it == s_ofstreamMap.end()) {
        return;
    }

    it->second << fileMsg << std::endl;
}
void Logger::setLockingEnabled(bool enabled)
{
    s_lockEnabled.store(enabled);
}
void Logger::setConsoleConciseEnabled(bool enabled)
{
    s_consoleConciseEnabled = enabled;
}
Logger LogDebug(const std::string& channel, const std::source_location& location)
{
    return Logger(LogLevel::DEBUG, channel, location);
}
Logger LogInfo(const std::string& channel, const std::source_location& location)
{
    return Logger(LogLevel::INFO, channel, location);
}
Logger LogWarn(const std::string& channel, const std::source_location& location)
{
    return Logger(LogLevel::WARN, channel, location);
}
Logger LogError(const std::string& channel, const std::source_location& location)
{
    return Logger(LogLevel::ERROR, channel, location);
}
FunctionTracer::FunctionTracer(LogLevel level, const std::string& channel, const std::source_location& location)
    : m_level(level), m_channel(channel), m_location(location)
{
    Logger(level, channel, location) << "[ENTER][" << m_location.function_name() << "]";
}
FunctionTracer::~FunctionTracer()
{
    Logger(m_level, m_channel, m_location) << "[EXIT][" << m_location.function_name() << "]";
}
} // namespace logger