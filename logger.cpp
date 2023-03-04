#include "logger.h"
#include "logger.h"
#include <cstdio>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <string>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace neapu;

#define ToCString str().c_str
#define IsEmpty empty

int Logger::m_nLogLevel = LM_NOLOG;
int Logger::m_nPrintLevel = LM_DEBUG;
FILE* Logger::m_pFile = nullptr;
Logger::String Logger::m_strLogDate;
std::mutex Logger::m_fileMutex;
Logger::String Logger::m_strLogPath;

Logger::Logger(int level)
    : m_nLevel(level)
{
}

Logger::~Logger()
{
    std::time_t now_c = time(nullptr);
    if (m_nLevel <= m_nLogLevel) {
        if (openFile()) {
            std::unique_lock<std::mutex> locker(m_fileMutex);
            fprintf(m_pFile, "%s%s%s\n", GetTime().c_str(), getLevelFlag(m_nLevel, false), m_data.ToCString());
            fflush(m_pFile);
        }
    }
    if (m_nLevel <= m_nPrintLevel) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwWritten;
        StringStream rst;
        rst << GetTime() << getLevelFlag(m_nLevel, false) << m_data.ToCString();
        WriteConsoleA(hConsole, rst.ToCString(), rst.str().length(), &dwWritten, nullptr);
#else
        fprintf(stderr, "%s%s%s\n", GetTime().c_str(), getLevelFlag(m_nLevel, false), m_data.ToCString());
#endif
    }
}

void Logger::setLogLevel(int nLogLevel, const String& strLogPath)
{
    if (strLogPath.IsEmpty()) return;
    m_strLogPath = strLogPath;
    if (m_strLogPath.back() != '/') {
        m_strLogPath.push_back('/');
    }
    const char* path = m_strLogPath.c_str();
#ifdef _WIN32
    if (0 != _access(path, 0)) {
        if (0 != _mkdir(path))
#else
    if (0 != access(path, 0)) {
        if (0 != mkdir(path, 0744))
#endif
        {
            perror("mkdir");
            return;
        }
    }
    m_nLogLevel = nLogLevel;
}

void Logger::setPrintLevel(int nPrintLevel)
{
    m_nPrintLevel = nPrintLevel;
}

void neapu::Logger::setConsoleChcp()
{
    system("chcp 65001");
}

bool Logger::openFile()
{
    std::time_t now_c = time(nullptr);
    char temp[64];
    strftime(temp, 64, "%Y-%m-%d", localtime(&now_c));
    if (m_pFile) {
        if (m_strLogDate == temp) return true;
    }
    m_strLogDate = temp;
    char szNewFile[128];
    sprintf(szNewFile, "%s%s.log", m_strLogPath.c_str(), temp);
    if (m_pFile) fclose(m_pFile);
    m_pFile = fopen(szNewFile, "a");
    if (!m_pFile) return false;
    return true;
}

const char* Logger::getLevelFlag(int level, bool bColor)
{
    if (bColor) {
        switch (level) {
        case LM_DEADLY: return "\033[0;35;40m[Deadly]\033[0m";
        case LM_ERROR: return "\033[0;31;40m[Error]\033[0m";
        case LM_INFO: return "\033[0;32;40m[Info]\033[0m";
        case LM_DEBUG: return "\033[0;34;40m[Debug]\033[0m";
        }
    } else {
        switch (level) {
        case LM_DEADLY: return "[Deadly]";
        case LM_ERROR: return "[Error]";
        case LM_INFO: return "[Info]";
        case LM_DEBUG: return "[Debug]";
        }
    }

    return "[Unknow]";
}

Logger::String Logger::GetTime()
{
    using namespace std::chrono;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    auto microsecond = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() % 1000000;

#ifdef _WIN32
    struct std::tm valtm;
    localtime_s(&valtm, &tt);
    auto pValtm = &valtm;
#else
    auto pValtm = localtime(&tt);
#endif

    std::stringstream ss;
    ss << "[" << std::put_time(pValtm, "%F %X") << " " << std::setw(3) << std::setfill('0') << microsecond / 1000 << "]";
    return ss.str();
}