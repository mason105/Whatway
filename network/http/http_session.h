#ifndef _HTTP_SESSION_
#define _HTTP_SESSION_


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/object_pool.hpp>


#include "http_message.h"
#include "ThreadSafeQueue/job_queue.h"
#include "network/isession.h"

class IMessage;


class http_session : public boost::enable_shared_from_this<http_session>, public ISession
{
public:
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::asio::io_service ios_type;

	typedef ios_type::strand strand_type;
	typedef job_queue<IMessage*> queue_type;
	typedef boost::object_pool<IMessage*> object_pool_type;

private:
	socket_type socket_;
	strand_type strand_;
	queue_type& queue_;
	static object_pool_type msg_pool_;


public:
	http_session(ios_type& ios, queue_type& q);

	socket_type& socket();
	ios_type& io_service();

	void start();
	void close();
	
	IMessage* create_request();
	void read(IMessage* req);

	void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	
	void write(IMessage* resp);
	void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
	void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
};

typedef boost::shared_ptr<http_session> http_session_ptr;

#endif // _TCP_SESSION_

