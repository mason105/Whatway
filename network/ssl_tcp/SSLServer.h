#ifndef _SSL_SERVER_H_
#define _SSL_SERVER_H_


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/factory.hpp>
#include <boost/asio/ssl.hpp>

#include "./network/ssl_tcp/SSLSession.h"
#include "./network/io_service_pool.h"

#include "./config/ConfigManager.h"

class SSLServer
{
public:
	typedef SSLSession::ios_type ios_type;
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	typedef SSLSession::queue_type queue_type;

private:
	io_service_pool& ios_pool_;
	acceptor_type acceptor_;
	queue_type& queue_;
	boost::asio::ssl::context context_;

	
public:
	SSLServer( unsigned short port, queue_type& q, int msgType, int n=4);
	SSLServer(io_service_pool& ios, unsigned short port, queue_type& q, int msgType);
	void start();
	void run();
	void stop();

	bool verify_certificate(bool preverified,      boost::asio::ssl::verify_context& ctx);
	std::string get_password();
	
	int m_msgType;

private:
	void start_accept();
	void accept_handler(const boost::system::error_code& error, ISession* session);
	
};

#endif
