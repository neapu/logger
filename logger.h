#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <string>
#include <mutex>
#define LM_NOLOG 0
#define LM_DEADLY 1
#define LM_ERROR 2
#define LM_INFO 3
#define LM_DEBUG 4

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define LOG(x) Logger(x) << "[" << __FILENAME__ << ":" << __LINE__ << "][" << __FUNCTION__ << "]"
#define LOG_DEBUG LOG(LM_DEBUG)
#define LOG_INFO LOG(LM_INFO)
#define LOG_ERROR LOG(LM_ERROR)
#define LOG_DEADLY LOG(LM_DEADLY)

using String = std::string;

class Logger {
public:
    Logger(int level);
    ~Logger();

    static void setLogLevel(int nLogLevel, const String& strLogPath);
    static void setPrintLevel(int nPrintLevel);

    Logger& operator<<(const char str[]);
    Logger& operator<<(const String& str);
    Logger& operator<<(const double n);
    Logger& operator<<(const int n);
    Logger& operator<<(const long long int n);
    Logger& operator<<(const unsigned int n);
    Logger& operator<<(const unsigned long long int n);

    template <typename T>
    Logger& operator<<(const T& data)
    {
        m_data += (data.ToString());
        return (*this);
    }

private:
    static bool openFile();
    static const char* getLevelFlag(int level, bool bColor);

private:
    int m_nLevel;
    String m_data;

    static int m_nLogLevel;
    static int m_nPrintLevel;
    static FILE* m_pFile;
    static String m_strLogDate;
    static std::mutex m_fileMutex;
    static String m_strLogPath;
};
#endif // __LOGGER_H__