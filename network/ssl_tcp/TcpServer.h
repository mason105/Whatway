#ifndef _TCP_SERVER_
#define _TCP_SERVER_



#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/factory.hpp>

#include "network/ssl_tcp/tcpsession.h"
#include "network/io_service_pool.h"


/*
session的释放可以参考
http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp03/ssl/server.cpp

http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp03/http/server2/connection.hpp
*/
class TcpServer
{
public:
	typedef TcpSession::ios_type ios_type;
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	typedef boost::asio::ip::tcp tcp_type;
	typedef TcpSession::queue_type queue_type;

private:
	io_service_pool& ios_pool_;
	acceptor_type acceptor_;
	queue_type& queue_;

	




public:
	TcpServer(io_service_pool& ios, unsigned short port, queue_type& q, int m_msgType);
	TcpServer(unsigned short port, queue_type& q, int m_msgType, int n=4);
	

	


private:
	void start_accept();
	void accept_handler(const boost::system::error_code& error, TcpSession * session);
	

public:
	void start();
	void run();
	void stop();
	
	int m_msgType;
};

#endif
