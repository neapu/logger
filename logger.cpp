#include "logger.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>
#include <memory>
#include <vector>
#include <unistd.h>

int Logger::m_nPrintLevel = LM_DEBUG;
int Logger::m_nLogLevel = LM_NOLOG;
std::string Logger::m_strLogPath;
FILE* Logger::m_pFile = nullptr;
std::string Logger::m_strLogDate;

inline std::string currentDateTime(const std::string& format) {
    time_t _time = time(nullptr);
    struct tm* _tm = localtime(&_time);
    char buf[1024];
    strftime(buf, 1024, format.c_str(), _tm);
    return std::string(buf, strlen(buf));
}

template<typename ... Args>
inline std::string formatString(const std::string& format, Args ... args)
{
    auto size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

void mkdirs(const char *muldir) 
{
    int i,len;
    char str[512];    
    strncpy(str, muldir, 512);
    len=strlen(str);
    for( i=0; i<len; i++ )
    {
        if( str[i]=='/' )
        {
            str[i] = '\0';
            if( access(str,0)!=0 )
            {
                mkdir( str, 0775 );
            }
            str[i]='/';
        }
    }
    if( len>0 && access(str,0)!=0 )
    {
        mkdir( str, 0775 );
    }
    return;
}

void Logger::init(int nPrintLevel, int nLogLevel, std::string strLogPath)
{
	m_nPrintLevel = nPrintLevel;
	m_nLogLevel = nLogLevel;
	m_strLogPath = strLogPath;
}

void Logger::clean()
{
	if (m_pFile) {
		fclose(m_pFile);
		m_pFile = nullptr;
	}
}

Logger::~Logger()
{
	if (m_strLogContent.empty())return;

	if (m_nLevel > LM_NOLOG && m_nLevel <= m_nLogLevel) {
		if (openLogFile()) {
			fwprintf(m_pFile, L"[%s]%s%s\n", currentDateTime("%Y-%m-%d %H:%M:%S").c_str(), getLevelFlag(m_nLevel, false), m_strLogContent.c_str());
			fflush(m_pFile);
		}
	}

	if (m_nLevel > LM_NOLOG && m_nLevel <= m_nPrintLevel) {
		fwprintf(stderr, L"[%s]%s%s\n", currentDateTime("%Y-%m-%d %H:%M:%S").c_str(), getLevelFlag(m_nLevel, true), m_strLogContent.c_str());
		fflush(m_pFile);
	}
}

Logger& Logger::operator<<(const std::string& str)
{
	m_strLogContent.append(str);
	return *this;
}

Logger& Logger::operator<<(const int _number)
{
    m_strLogContent.append(std::to_string(_number));
    return *this;
}

Logger& Logger::operator<<(const unsigned int _number)
{
    m_strLogContent.append(std::to_string(_number));
    return *this;
}


bool Logger::openLogFile()
{
	auto _t = currentDateTime("%Y-%m-%d");
	

	if (m_pFile) {
		if (m_strLogDate == _t) {
			return true;
		}
		fclose(m_pFile);
	}

    m_strLogDate = _t;
	
	if (!m_strLogPath.empty()) {
        if(access(m_strLogPath.c_str(), 0)!=0){
            mkdirs(m_strLogPath.c_str());
        }
		std::string strFile = formatString("%s/%s.log", m_strLogPath.c_str(), m_strLogDate.c_str());
		m_pFile = std::fopen(strFile.c_str(), "a+");
		if (m_pFile)return true;
	}
	return false;
}

const char* Logger::getLevelFlag(int nLevel, bool bColor)
{
	if(bColor) {
        switch (nLevel) {
        case LM_DEADLY: return "\033[0;35;40m[Deadly]\033[0m";
        case LM_ERROR:  return "\033[0;31;40m[Error]\033[0m";
        case LM_INFO:   return "\033[0;32;40m[Info]\033[0m";
        case LM_DEBUG:  return "\033[0;34;40m[Debug]\033[0m";
        }
    } else {
        switch (nLevel) {
        case LM_DEADLY: return "[Deadly]";
        case LM_ERROR:  return "[Error]";
        case LM_INFO:   return "[Info]";
        case LM_DEBUG:  return "[Debug]";
        }
    }
    
    return "[Unknow]";
}
