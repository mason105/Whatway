#ifndef TCP_CLIENT_SYNC_H
#define TCP_CLIENT_SYNC_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "network/ssl_tcp/MsgHeader.h"
#include "network/ssl_tcp/CustomMessage.h"

#include "business/IBusiness.h"

/*
问题一：超时
问题二：循环读写
问题三：运行时间
*/
class CTCPClientSync : public IBusiness
{
public:
	CTCPClientSync();
	~CTCPClientSync(void);

	virtual bool CreateConnect();
	virtual bool Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg);
	virtual void CloseConnect();

	//void SetConnectTimeout(int seconds);
	//void SetReadWriteTimeout(int seconds);
	// 心跳功能，由于后台业务不同，所以封装
	bool HeartBeat();
	

private:
	bool Write(CustomMessage * pReq);
	bool WriteMsgHeader(CustomMessage * pReq);
	bool WriteMsgContent(CustomMessage * pReq);

	bool Read(CustomMessage * pRes);
	bool ReadMsgHeader(CustomMessage * pRes);
	bool ReadMsgContent(CustomMessage * pRes);

	int Send(const char * buf, int len, int flags);
	int Recv(char* buf, int len, int flags);
	//std::string decompress(std::string data);

private:
	SOCKET sockfd;
	
};
#endif
