#ifndef _SSL_SESSION_H_
#define _SSL_SESSION_H_

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/asio/ssl.hpp>


#include "ThreadSafeQueue/job_queue.h"
#include "business/IBusiness.h"
//#include "network/ISession.h"



class IMessage;

class SSLSession : public boost::enable_shared_from_this<SSLSession>
{
public:
	
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_type;
	typedef boost::asio::io_service ios_type;

	typedef ios_type::strand strand_type;
	typedef job_queue<IMessage*> queue_type;
	

private:
	socket_type socket_;
	strand_type strand_;
	queue_type& queue_;
	//static object_pool_type msg_pool_;


public:
	SSLSession(ios_type& ios, queue_type& q, int msgType, boost::asio::ssl::context& context);
	~SSLSession();

	socket_type::lowest_layer_type& socket();
	

	ios_type& io_service();

	virtual void start();
	virtual void close();

	
	void handle_handshake(const boost::system::error_code& error);

	virtual IMessage* create_request();
	virtual void read();
	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	
	virtual void write(IMessage* resp);
	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
	

	// 柜台连接
	IBusiness * counterT2;
	IBusiness * counterSzkingdom;
	IBusiness * counterApex;
	IBusiness * counterAGC;
	IBusiness * counterXinYi;

	// 消息类型
	int m_msgType;

	
	// 关闭柜台连接
	void CloseCounterConnect();
	// 得到柜台连接
	IBusiness * GetCounterConnect(int counterType);
};

typedef boost::shared_ptr<SSLSession> SSLSessionPtr;

#endif // _TCP_SESSION_

