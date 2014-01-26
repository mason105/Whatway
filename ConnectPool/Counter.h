#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <string>
#include "common.h"


class Counter
{
public:
	Counter(void);
	~Counter(void);

	std::string m_sIP;
	int m_nPort;
	std::string m_sServerName;
	std::string m_sUserName;
	std::string m_sPassword;
	std::string m_sReq;
	std::string m_sRes;

	COUNTER_TYPE m_eCounterType;
	int m_nConnectTimeout; // 柜台连接超时时间
	int m_nIdleTimeout; // 柜台连接后，没有请求，自动中断连接时间
	int m_nRecvTimeout; // 请求处理超时时间

	std::string m_sGydm; // 顶点柜台，柜员代码
	std::string m_sWtfs_mobile;
	std::string m_sWtfs_web;

};
#endif
