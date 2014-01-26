#include "stdafx.h"

#include "CacheData.h"

#include <process.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "./output/FileLog.h"


CCacheData g_CacheData;

CCacheData::CCacheData(void)
{
}

CCacheData::~CCacheData(void)
{
}

void CCacheData::Start()
{
	m_bRunning = TRUE;

	m_hThread = NULL;
	m_hThread = (void*) _beginthread(CCacheData::Run, 0, this);
}

void CCacheData::Run(void* params)
{
	gFileLog::instance().Log("cache清除线程启动");

	CCacheData * pThis = (CCacheData*)params;

	while(pThis->m_bRunning)
	{
		boost::posix_time::ptime now =  boost::posix_time::second_clock::local_time();

		boost::posix_time::time_duration td = now.time_of_day();

		// 清除缓存数据
		if (td.hours() == 8 && (td.minutes() >=30  && td.minutes() <31))
		{
			gFileLog::instance().Log("cache clean");

			std::map<std::string, std::string>::iterator it = pThis->m_mapCacheData.begin();
			for(; it != pThis->m_mapCacheData.end(); it++)
			{
				it->second = "";
			}
		}

		Sleep(1000*20); // 10秒
	}
}

void CCacheData::Stop()
{
	m_bRunning = false;

	if (m_hThread != NULL)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}
