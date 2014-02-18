#ifndef _CONNECT_
#define _CONNECT_

#include "stdafx.h"

#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "common.h"


// 恒生T2
#include "business/hundsun_t2/t2sdk_interface.h"

// 恒生CommX
#include "business/hundsun_com/CComm.h"






#include "Counter.h"


class Connect
{
public:
	Connect(int ConnectNo, Counter counter);
	~Connect(void);

	void init();

	int m_nID;
	Counter m_Counter ;


	std::string m_sServerInfo;
	std::string GetConnectInfo();

	time_t prev; // 创建或前一次使用的时间，用于判断是否因为长时间没有操作造成的超时
	bool IsTimeout();

	bool CreateConnect();
	bool ReConnect();
	void CloseConnect();


	// 金证
	void * handle; // 柜台连接句柄
	bool CreateConnectKCBP();

	// 恒生T2
	CConfigInterface * lpConfig;
	CConnectionInterface * lpConnection;
	bool CreateConnectT2();

	// 恒生CommX
	CComm * m_pComm;
	bool CreateConnectComm();

	// 顶点
	long m_hHandle;
	bool CreateConnectDingDian();	

	

	

};
#endif
