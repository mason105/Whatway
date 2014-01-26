#include "ConnectPool.h"
#include "./config/ConfigManager.h"

#include "./output/FileLog.h"



ConnectPool::ConnectPool(std::vector<Counter> vCounter)
{
	m_nID = 0;

	m_vCounter = vCounter;

	m_bCreatePool = false;
}

ConnectPool::~ConnectPool(void)
{
	CloseConnPool();
}

bool ConnectPool::CreateConnPool()
{
		std::string msg = "开始创建连接池";
		gFileLog::instance().Log(msg);

	bool bRet = false;

	
	m_bCreatePool = true;

	// 初始化连接数=柜台服务器数 * m_nConnPoolMin
	// 假设服务器有4个, m_nConnPoolMin=2, 创建后s1, s2, s3, s4, s1,s2,s3,s4

	for (int i=0; i < gConfigManager::instance().m_nConnectPoolMin; i++)
	{
		for (std::vector<Counter>::iterator pos = m_vCounter.begin(); pos != m_vCounter.end(); pos++)
		{
			
			Connect * pConn = new Connect(m_nID, *pos);
			if (pConn->CreateConnect())
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
		std::string msg = "建立连接池失败";
		gFileLog::instance().Log(msg);
		

		bRet = false;
	}
	else
	{
		std::string msg = "建立连接池成功";
		gFileLog::instance().Log(msg);


		bRet = true;
	}


	return bRet;
}


bool ConnectPool::IncreaseConnPool()
{
	bool bRet = false;

	int nOldSize = m_pool.queue_.size();



	// 增长数=柜台服务器数 * m_nConnPoolIncrease
	for (int i=0; i < gConfigManager::instance().m_nConnectPoolIncrease; i++)
	{
		for (std::vector<Counter>::iterator pos = m_vCounter.begin(); pos != m_vCounter.end(); pos++)
		{

			Connect * pConn = new Connect(m_nID, *pos);
			if (pConn->CreateConnect())
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
			
		std::string msg = "扩充连接池失败";
		gFileLog::instance().Log(msg);

		bRet = false;
	}
	else
	{
		std::string msg = "扩充连接池成功";
		gFileLog::instance().Log(msg);


		bRet = true;
	}

	return bRet;
}



// 获取连接的遍历采用循环方法，不采用随机方法
 Connect* ConnectPool::GetConnect()
{
	std::string msg;

	if (m_pool.queue_.empty())
	{
		if (!IncreaseConnPool())
			return NULL;
	}

	Connect *pConn = m_pool.pop();
	if (pConn == NULL)
	{
		msg = "获取连接，失败";
		gFileLog::instance().Log(msg);

		return NULL;
	}


	if (pConn->IsTimeout())
	{
		if (!pConn->ReConnect())
		{
			msg = "获取连接超时重连失败" + pConn->GetConnectInfo();
			gFileLog::instance().Log(msg);

			return NULL;
		}
	}

	msg = "获取连接成功, " + pConn->GetConnectInfo();
	gFileLog::instance().Log(msg);

	return pConn;
}

void ConnectPool::PushConnect(Connect* pConn)
{
	if (pConn == NULL)
		return;

	std::string msg = "释放连接, " + pConn->GetConnectInfo();
	gFileLog::instance().Log(msg);

	msg = "释放连接: 归还前大小" + boost::lexical_cast<std::string>(m_pool.queue_.size());
	gFileLog::instance().Log(msg);

	m_pool.push(pConn);

	msg = "释放连接: 归还后大小" + boost::lexical_cast<std::string>(m_pool.queue_.size());
	gFileLog::instance().Log(msg);
}

void ConnectPool::CloseConnPool()
{
	std::string msg = "关闭连接池";
	gFileLog::instance().Log(msg);

	m_bCreatePool = false;

	m_pool.stop();

	/*
	方案一
	Connect * pConn = m_pool.pop();
	*/

	// 方案二
	for (std::deque<Connect*>::iterator pos = m_pool.queue_.begin(); pos != m_pool.queue_.end(); pos++)
	{
		Connect * pConn = *pos;

		if (pConn != NULL)
		{
			pConn->CloseConnect();
			delete pConn;
			pConn = NULL;
		}
	}
	
}

bool ConnectPool::IsCreatePool()
{
	return m_bCreatePool;
}