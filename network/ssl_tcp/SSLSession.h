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

#include "network/ISession.h"


class CustomMessage;


class SSLSession : public boost::enable_shared_from_this<SSLSession>, public ISession
{
public:
	
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_type;
	typedef boost::asio::io_service ios_type;

	typedef ios_type::strand strand_type;
	typedef job_queue<CustomMessage*> queue_type;
	

private:
	socket_type socket_;
	strand_type strand_;
	queue_type& queue_;
	//static object_pool_type msg_pool_;


public:
	SSLSession(ios_type& ios, queue_type& q,boost::asio::ssl::context& context);

	socket_type::lowest_layer_type& socket();
	

	ios_type& io_service();

	void start();
	void close();

	

	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* req);
	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* req);
	
	virtual void write(CustomMessage* resp);
	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* resp);
	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* resp);


private:
	CustomMessage* create_request();
	
	void read();
	void handle_handshake(const boost::system::error_code& error);

	
};

typedef boost::shared_ptr<SSLSession> SSLSessionPtr;

#endif // _TCP_SESSION_

