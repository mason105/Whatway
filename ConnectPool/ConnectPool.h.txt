#ifndef _CONNECT_POOL_H_
#define _CONNECT_POOL_H_

#include <vector>
#include "connect.h"
#include "common.h"
#include "Counter.h"
#include "ThreadSafeQueue/job_queue.h"

class ConnectPool
{
public:
	ConnectPool(std::vector<Counter> vCounter);
	~ConnectPool(void);


	// 直接整合std::deque比较好

	typedef job_queue<Connect*> conn_queue_type;

private:
	conn_queue_type m_pool;

	std::vector<Counter> m_vCounter;
	int m_nID; // 连接序列号
	int m_nConnCount; // 连接数

public:
	bool CreateConnPool();
	bool IncreaseConnPool();
	void CloseConnPool();

	Connect* GetConnect();
	void PushConnect(Connect * pConn);


	bool m_bCreatePool;
	bool IsCreatePool();

};
#endif
