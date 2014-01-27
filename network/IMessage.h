#ifndef _IMESSAGE_H_
#define _IMESSAGE_H_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "log/tradelog.pb.h"

#define MSG_TYPE_HTTP 0
#define MSG_TYPE_TCP_OLD 1
#define MSG_TYPE_SSL_PB 2
#define MSG_TYPE_TCP_NEW 3
//#define MSG_TYPE_SSL_NEW 4


class ISession;

class IMessage
{
public:
	IMessage();
	~IMessage(void);

	// 业务处理生成的日志
	// 日志的处理
	void SetRecvTime();
	void SetSendTime();

	Trade::TradeLog log;
	void Log(Trade::TradeLog::LogLevel logLevel, std::string sysNo, std::string sysVer, std::string busiType, std::string funcId, std::string account, std::string clientIp, std::string request, std::string response, int status, std::string errCode, std::string errMsg, std::string beginTime, int runtime, std::string gatewayIp, std::string gatewayPort, std::string counterIp, std::string counterPort, std::string counterType);

	ISession* session;
	void SetSession(ISession* session);
	ISession* GetSession();

	size_t m_MsgHeaderSize;
	std::vector<char> m_MsgHeader;
	size_t GetMsgHeaderSize();
	std::vector<char>& GetMsgHeader();
	

	std::vector<char> m_MsgContent;
	size_t GetMsgContentSize();
	std::vector<char>& GetMsgContent();
	

	int msgType;
	
	

	// 删除消息指针
	void destroy();

	// 发送消息
	void SetMsgContent(std::string content);
	void SetMsgHeader(std::vector<char> header);

	// 接收消息
	virtual bool DecoderMsgHeader()=0;
	std::string GetMsgContentString();
};

#endif
