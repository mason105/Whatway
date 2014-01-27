#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#include <vector>

#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "MsgHeader.h"
//#include "tcpsession.h"

#include "network/imessage.h"

/*
独立出来的原因，
1.以后可以改由内存池分配
2.异步调用的时候适合放入队列
*/




class CustomMessage : public IMessage
{
public:
	CustomMessage();

private:
	// 消息头
	struct MsgHeader m_MsgHeader;
	//std::vector<char> m_MsgHeader;

	// 消息内容
	std::vector<char> m_MsgContent;

	
	

public:
	
	

	
	void SetMsgHeader(unsigned char MsgType, int FunctionNo, unsigned char zip=0);
	bool ParseMsgHeader();
	PMSG_HEADER GetMsgHeader();
	size_t GetMsgHeaderSize();


	void SetMsgContent(std::string content);
	char * GetMsgContent();
	size_t GetMsgContentSize();
	std::string GetMsgContentString();

	void destroy();
	

	// 日志的处理
	void SetRecvTime();
	void SetSendTime();

	

	

	void Log(Trade::TradeLog::LogLevel logLevel, std::string sysNo, std::string sysVer, std::string busiType, std::string funcId, std::string account, std::string clientIp, std::string request, std::string response, int status, std::string errCode, std::string errMsg, std::string beginTime, int runtime, std::string gatewayIp, std::string gatewayPort, std::string counterIp, std::string counterPort, std::string counterType);
};



#endif