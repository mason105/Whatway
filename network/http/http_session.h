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

class http_message;
typedef http_message http_request;
typedef http_message http_response;
typedef http_request* http_request_ptr;
typedef http_response* http_response_ptr;

class http_session : public boost::enable_shared_from_this<http_session>, public ISession
{
public:
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::asio::io_service ios_type;

	typedef ios_type::strand strand_type;
	typedef job_queue<http_request_ptr> queue_type;
	typedef boost::object_pool<http_message> object_pool_type;

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
	void write(http_response_ptr resp);

private:
	http_request_ptr create_request();
	void read(http_request_ptr req);

	void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, http_request_ptr req);
	void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, http_request_ptr req);

	void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, http_response_ptr resp);
	void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, http_response_ptr resp);
};

typedef boost::shared_ptr<http_session> http_session_ptr;

#endif // _TCP_SESSION_

