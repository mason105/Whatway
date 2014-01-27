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

#include "./output/FileLog.h"


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
	  TcpServer(io_service_pool& ios, unsigned short port, queue_type& q):
		
		ios_pool_(ios),
		queue_(q),
		acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		{
			acceptor_.set_option(acceptor_type::reuse_address(true));
			start_accept();
		}


	TcpServer(unsigned short port, queue_type& q, int n=4):
	 
	  ios_pool_(*boost::factory<io_service_pool*>()(n)),
	  queue_(q),
	  acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	  {
		  acceptor_.set_option(acceptor_type::reuse_address(true));
		  start_accept();
	  }

	


private:
	void start_accept()
	{
	/*	
		tcp_session_old_ptr session = 
		//tcp_session_old * session = new tcp_session_old(ios_pool_.get(), queue_);

		acceptor_.async_accept(session->socket(), 
			boost::bind(&tcp_server_old::accept_handler, 
			this, 
			boost::asio::placeholders::error, 
			session));

*/
		//session_.reset(new TcpSession(log_, ios_pool_.get(), queue_));

		boost::shared_ptr<TcpSession> session = boost::factory<boost::shared_ptr<TcpSession>>()(ios_pool_.get(), queue_);

		acceptor_.async_accept(session->socket(), 
			boost::bind(&TcpServer::accept_handler, 
			this, 
			boost::asio::placeholders::error, 
			session));

	}

	void accept_handler(const boost::system::error_code& error, boost::shared_ptr<TcpSession> session)
	{
		start_accept();

		if (error)
		{
			gFileLog::instance().Log("accept´íÎó£¬´íÎó´úÂë:" + boost::lexical_cast<std::string>(error.value()) + "£¬´íÎóÏûÏ¢:" + error.message());

			session->close();
			return;
		}

		session->start();
	}

public:
	void start()
	{
		ios_pool_.start();
	}

	void run()
	{
		ios_pool_.run();
	}

	void stop()
	{
		ios_pool_.stop();
	}

};

#endif
