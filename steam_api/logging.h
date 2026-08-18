#ifndef _LOGGING_H
#define _LOGGING_H

#ifdef _WIN32
#pragma once
#endif

#include <mutex>
#include "revCommon.h"

class CLoggingSystem
{
public:
	CLoggingSystem(char* strFile);
	~CLoggingSystem();

	// write log
	void Write(const char* pszFormat, ...);

	// clear it
	void Clear();

private:
	FILE*		m_pLogFile;
	char		m_szFileName[255]; // gcc clang 255, msvc 260
	std::mutex	m_LogMutex;
};

#endif