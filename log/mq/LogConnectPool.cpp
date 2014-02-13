#include "stdafx.h"

#include "LogConnectPool.h"
#include "LogConnect.h"

#include "./config/ConfigManager.h"
#include "./output/FileLog.h"


LogConnectPool::LogConnectPool(void)
{
	m_nID = 0;
	m_nConnCount = 0;
	logFileName = "分布式日志";
}


LogConnectPool::~LogConnectPool(void)
{
}

BOOL LogConnectPool::CreateConnectPool()
{
	std::string msg = "开始创建日志连接池";
	gFileLog::instance().Log(msg, logFileName);

	bool bRet = false;

	
	//m_bCreatePool = true;

	// 初始化连接数=柜台服务器数 * m_nConnPoolMin
	// 假设服务器有4个, m_nConnPoolMin=2, 创建后s1, s2, s3, s4, s1,s2,s3,s4

	for (int i=0; i < gConfigManager::instance().m_nLogMqMin; i++)
	{
		for (std::vector<std::string>::iterator pos = gConfigManager::instance().m_vLogMqServer.begin(); pos != gConfigManager::instance().m_vLogMqServer.end(); pos++)
		{
			
			std::string server = *pos;

			LogConnect * pConn = new LogConnect();
			if (pConn->Connect(server))
			{
				m_pool.push(pConn);
				m_nID++;
			}
			else
			{
				delete pConn;
			}

		} // end for 柜台服务器数
	} // end for 初始次数
	

	m_nConnCount = m_pool.queue_.size();

	if (m_nConnCount == 0)
	{
		std::string msg = "建立日志连接池失败";
		gFileLog::instance().Log(msg, logFileName);
		

		bRet = false;
	}
	else
	{
		std::string msg = "建立日志连接池成功";
		gFileLog::instance().Log(msg, logFileName);


		bRet = true;
	}


	return bRet;
}

BOOL LogConnectPool::IncreaseConnPool()
{
	BOOL bRet = FALSE;

	int nOldSize = m_pool.queue_.size();



	// 增长数=柜台服务器数 * m_nConnPoolIncrease
	for (int i=0; i < gConfigManager::instance().m_nLogMqIncrease; i++)
	{
		for (std::vector<std::string>::iterator pos = gConfigManager::instance().m_vLogMqServer.begin(); pos != gConfigManager::instance().m_vLogMqServer.end(); pos++)
		{
			std::string server = *pos;

			LogConnect * pConn = new LogConnect();
			if (pConn->Connect(server))
			{
				m_pool.push(pConn);
				m_nID++;
			}
			else
			{
				delete pConn;
			}

		} // end for 柜台服务器数
	} // end for 增长次数
		

	m_nConnCount = m_pool.queue_.size();

	if (m_nConnCount == nOldSize)
	{
			
		std::string msg = "扩充日志连接池失败";
		gFileLog::instance().Log(msg, logFileName);

		bRet = FALSE;
	}
	else
	{
		std::string msg = "扩充日志连接池成功";
		gFileLog::instance().Log(msg, logFileName);


		bRet = TRUE;
	}

	return bRet;
}

void LogConnectPool::CloseConnectPool()
{
	std::string msg = "关闭连接池";
	gFileLog::instance().Log(msg, logFileName);

	//m_bCreatePool = false;

	m_pool.stop();

	/*
	方案一
	Connect * pConn = m_pool.pop();
	*/

	// 方案二
	for (std::deque<LogConnect*>::iterator pos = m_pool.queue_.begin(); pos != m_pool.queue_.end(); pos++)
	{
		LogConnect * pConn = *pos;

		if (pConn != NULL)
		{
			pConn->Close();
			delete pConn;
			pConn = NULL;
		}
	}
}




LogConnect* LogConnectPool::GetConnect()
{
	std::string msg;

	if (m_pool.queue_.empty())
	{
		if (!IncreaseConnPool())
			return NULL;

	}

	LogConnect *pConn = m_pool.pop();
	if (pConn == NULL)
	{
		msg = "获取连接，失败";
		gFileLog::instance().Log(msg, logFileName);

		return NULL;
	}



	msg = "获取连接成功";
	gFileLog::instance().Log(msg, logFileName);

	return pConn;
}

void LogConnectPool::PushConnect(LogConnect * pConn)
{
	if (pConn == NULL)
		return;

	std::string msg = "释放连接, " ;//+ pConn->GetConnectInfo();
	gFileLog::instance().Log(msg, logFileName);

	msg = "释放连接: 归还前大小" + boost::lexical_cast<std::string>(m_pool.queue_.size());
	gFileLog::instance().Log(msg, logFileName);

	m_pool.push(pConn);

	msg = "释放连接: 归还后大小" + boost::lexical_cast<std::string>(m_pool.queue_.size());
	gFileLog::instance().Log(msg, logFileName);
}
