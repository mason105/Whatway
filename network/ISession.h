#ifndef _ISESSION_H_
#define _ISESSION_H_

#include <boost/asio.hpp>

#include "business/IBusiness.h"
//#include "network/CustomMessage.h"

class IMessage;

class ISession
{
public:
	ISession(void);
	~ISession(void);

	// 柜台连接
	IBusiness * counterT2;
	IBusiness * counterSzkingdom;
	IBusiness * counterApex;
	IBusiness * counterAGC;
	IBusiness * counterXinYi;
	
	
	

	// 消息类型
	int m_msgType;

	virtual void start()=0;
	virtual void close()=0;
	
	virtual IMessage* create_request() = 0;
	virtual void read() = 0;
	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req) = 0;
	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req) = 0;
	
	virtual void write(IMessage* resp) = 0;
	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp) = 0;
	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp) = 0;

	// 创建柜台连接
	void CreateCounterConnect(Counter * counter);
	// 关闭柜台连接
	void CloseCounterConnect();
	// 得到柜台连接
	IBusiness * GetCounterConnect(int counterType);
};

#endif
