#ifndef _HTTP_SERVER_
#define _HTTP_SERVER_


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/factory.hpp>


#include "./network/http/http_session.h"
#include "./network/io_service_pool.h"


class http_server
{
public:
	typedef http_session::ios_type ios_type;
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	//typedef boost::asio::ip::tcp tcp_type;
	typedef http_session::queue_type queue_type;

private:
	io_service_pool& ios_pool_;
	acceptor_type acceptor_;
	queue_type& queue_;
	

public:
	http_server(unsigned short port, queue_type& q,int n=4):
		
	  ios_pool_(*boost::factory<io_service_pool*>()(n)),
	  queue_(q),
	  acceptor_(ios_pool_.get(),boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	  {
		  acceptor_.set_option(acceptor_type::reuse_address(true));

		  start_accept();
	  }

	  http_server(io_service_pool& ios, unsigned short port, queue_type& q):
		 
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
		//http_session_ptr session = boost::factory<http_session_ptr>()(ios_pool_.get(), queue_);
		http_session_ptr session = boost::factory<http_session_ptr>()(ios_pool_.get(), queue_);

		acceptor_.async_accept(session->socket(), 
			boost::bind(&http_server::accept_handler, this, boost::asio::placeholders::error, session));
	}

	void accept_handler(const boost::system::error_code& error, http_session_ptr session)
	{
		start_accept();

		if (error)
		{
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
