#include "logging.h"
#include "tier0/dbg.h"

// Constructor, open the logfile
CLoggingSystem::CLoggingSystem(char* strFile)
{
	strcpy(m_szFileName, strFile);
	m_pLogFile = fopen(m_szFileName, "w");

	if (!m_pLogFile)
		Warning("CLoggingSystem ERROR: failed opening target file %s\n", m_szFileName);
}

// Destructor, close if logfile is opened
CLoggingSystem::~CLoggingSystem()
{
	if (m_pLogFile)
	{
		fclose(m_pLogFile);
	}
}

// Write log info into the logfile, with printf like parameters support
void CLoggingSystem::Write(const char* cszFormat, ...)
{
	std::lock_guard<std::mutex> lock(m_LogMutex);

	if (!m_pLogFile)
		return;
 
	// Write the timestamp.
	std::time_t raw_time = std::time(nullptr);
	std::tm local_tm;

#if defined(_WIN32) || defined(_WIN64)
	// Windows thread-safe version (arguments reversed)
	localtime_s(&local_tm, &raw_time);
#else
	// Linux/POSIX thread-safe version
	localtime_r(&raw_time, &local_tm);
#endif

	fprintf(m_pLogFile, "%04d/%02d/%02d %02d:%02d:%02d\t",
		local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
		local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);

	// Write the actual log line.
	va_list ap;
	va_start(ap, cszFormat);
	vfprintf(m_pLogFile, cszFormat, ap);

	va_end(ap);

	fflush(m_pLogFile);
}

// Clear out the logfile
void CLoggingSystem::Clear()
{
	m_pLogFile = fopen(m_szFileName, "w");
	if (!m_pLogFile)
		return;

	fclose(m_pLogFile);
}
