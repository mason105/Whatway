#ifndef _TCP_SESSION_OLD_
#define _TCP_SESSION_OLD_



#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/object_pool.hpp>

#include "tcp_message_old.h"
#include "ThreadSafeQueue/job_queue.h"
#include "network/isession.h"

class IMessage;


class tcp_session_old : public boost::enable_shared_from_this<tcp_session_old>, public ISession
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
	tcp_session_old(ios_type& ios, queue_type& q);
	//~tcp_session_old();

	socket_type& socket();
	ios_type& io_service();

	virtual void start();
	virtual void close();
	


	virtual IMessage* create_request();
	virtual void read();

	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);

	virtual void write(IMessage* resp);
	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
};

typedef boost::shared_ptr<tcp_session_old> tcp_session_old_ptr;

#endif // _TCP_SESSION_

