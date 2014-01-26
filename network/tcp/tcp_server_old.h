#ifndef _TCP_SERVER_OLD_
#define _TCP_SERVER_OLD_

#include "./network/tcp/tcp_session_old.h"
#include "./network/io_service_pool.h"

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/factory.hpp>


#include "./output/FileLog.h"


class tcp_server_old
{
public:
	typedef tcp_session_old::ios_type ios_type;
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	//typedef boost::asio::ip::tcp tcp_type;
	typedef tcp_session_old::queue_type queue_type;

private:
	io_service_pool& ios_pool_;
	acceptor_type acceptor_;
	queue_type& queue_;

	tcp_session_old_ptr session_; 

	


public:
	tcp_server_old( unsigned short port, queue_type& q,int n=4):
	 
	  ios_pool_(*boost::factory<io_service_pool*>()(n)),
	  queue_(q),
	  acceptor_(ios_pool_.get(),boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	  {
		  acceptor_.set_option(acceptor_type::reuse_address(true));
		  start_accept();
	  }

	  tcp_server_old( io_service_pool& ios, unsigned short port, queue_type& q):
		
		ios_pool_(ios),
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
		tcp_session_old_ptr session = boost::factory<tcp_session_old_ptr>()(ios_pool_.get(), queue_);
		//tcp_session_old * session = new tcp_session_old(ios_pool_.get(), queue_);

		acceptor_.async_accept(session->socket(), 
			boost::bind(&tcp_server_old::accept_handler, 
			this, 
			boost::asio::placeholders::error, 
			session));

*/
		session_.reset(new tcp_session_old(ios_pool_.get(), queue_));

		acceptor_.async_accept(session_->socket(), 
			boost::bind(&tcp_server_old::accept_handler, 
			this, 
			boost::asio::placeholders::error, 
			session_));

	}

	void accept_handler(const boost::system::error_code& error, tcp_session_old_ptr session)
	//void accept_handler(const boost::system::error_code& error, tcp_session_old * session)
	{
		start_accept();

		if (error)
		{
			gFileLog::instance().Log("tcp accept_handler errcode:" + boost::lexical_cast<std::string>(error.value()) + "errmsg:" + error.message());

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
