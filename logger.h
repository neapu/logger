#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <sstream>
#include <string>
#include <mutex>
#include <string.h>
#define LM_NOLOG 0
#define LM_DEADLY 1
#define LM_ERROR 2
#define LM_INFO 3
#define LM_DEBUG 4

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define LOG(x) neapu::Logger(x) << "[" << __FILENAME__ << ":" << __LINE__ << "][" << __FUNCTION__ << "]"
#define LOG_DEBUG LOG(LM_DEBUG)
#define LOG_INFO LOG(LM_INFO)
#define LOG_ERROR LOG(LM_ERROR)
#define LOG_DEADLY LOG(LM_DEADLY)

namespace neapu {
class Logger {
    using String = std::string;

public:
    Logger(int level);
    ~Logger();

    static void setLogLevel(int nLogLevel, const String &strLogPath);
    static void setPrintLevel(int nPrintLevel);

    template <class T>
    Logger &operator<<(T &&t)
    {
        m_data << std::forward<T>(t);
        return *this;
    }

private:
    static bool openFile();
    static const char *getLevelFlag(int level, bool bColor);
    static String GetTime();

private:
    int m_nLevel;
    std::stringstream m_data;

    static int m_nLogLevel;
    static int m_nPrintLevel;
    static FILE *m_pFile;
    static String m_strLogDate;
    static std::mutex m_fileMutex;
    static String m_strLogPath;
};
} // namespace neapu

#endif // __LOGGER_H__