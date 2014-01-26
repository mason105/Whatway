#ifndef _CONNECT_MANAGER_H_
#define _CONNECT_MANAGER_H_

#include <map>
#include <string>

#include <mutex>

#include "common.h"
#include "BusinessSystem.h"
#include "BusinessType.h"
#include "Counter.h"
#include "connectpool.h"


class CConnectManager
{
public:
	CConnectManager(void);
	~CConnectManager(void);



	//保证线程安全?
	Connect* GetConnect(std::string sysNo, BUSI_TYPE busiType, std::string sBranchId);
	void PushConnect(Connect * pConn, std::string sysNo, BUSI_TYPE busiType, std::string sBranchId);
	void CloseConnPool();

	Counter* GetServer(std::string system, BUSI_TYPE business, std::string branch);
	int GetServerCount(std::string system, BUSI_TYPE business, std::string branch);
	

	// configmanager不需要定义std::map<std::string, BusinessSystem> systems;
	std::map<std::string, BusinessSystem> systems;

	//            系统编号         业务类型        营业部列表   连接池
	//std::map<std::string, std::map<enum, std::map<std::string, ConnectPool>> >
	// connectpool --> queue --> connect
};

extern CConnectManager g_ConnectManager;

#endif
