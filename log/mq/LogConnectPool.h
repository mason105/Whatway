#ifndef _MQ_MANAGER_H_
#define _MQ_MANAGER_H_

//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/noncopyable.hpp>
//#include <boost/thread.hpp>
#include <boost/thread/detail/singleton.hpp>


#include "ThreadSafeQueue/job_queue.h"

class LogConnect;

class LogConnectPool
{
public:
	LogConnectPool(void);
	~LogConnectPool(void);

	BOOL CreateConnectPool();
	BOOL IncreaseConnPool();
	LogConnect* GetConnect();
	void PushConnect(LogConnect * pConn);

	void CloseConnectPool();
	

private:
	job_queue<LogConnect*> m_pool;
	int m_nID; // 连接序列号
	int m_nConnCount; // 连接数
	std::string m_sLogFile;

	int m_nMin;
	int m_nIncrease;
	int m_nMax;
};

typedef boost::detail::thread::singleton<LogConnectPool> gLogConnectPool;

#endif
