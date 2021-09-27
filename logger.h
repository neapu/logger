#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <string>
#define LM_NOLOG	0
#define LM_NOTICE	1
#define LM_DEADLY	2
#define LM_ERROR	3
#define LM_INFO		4
#define LM_DEBUG	5

class Logger
{
public:
    static void init(int nPrintLevel, 
        int nLogLevel, 
        std::string strLogPath
    );
    static void clean();
    Logger(int nLevel) : m_nLevel(nLevel) {}
    virtual ~Logger();

    Logger& operator<<(const std::string& str);
private:
    static bool openLogFile();
    static const char* getLevelFlag(int nLevel, bool bColor);

private:
    static int m_nPrintLevel;
    static int m_nLogLevel;
    static std::string m_strLogPath;
    static FILE* m_pFile;
    static std::string m_strLogDate;

    std::string m_strLogContent;
    int m_nLevel;
};
#endif // __LOGGER_H__