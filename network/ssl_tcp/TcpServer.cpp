#include "stdafx.h"

#include "network/ssl_tcp/TcpServer.h"

#include "./output/FileLog.h"


TcpServer::TcpServer(io_service_pool& ios, unsigned short port, queue_type& q, int msgType):
	ios_pool_(ios),
	queue_(q),
	acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	,m_session()
{
	m_msgType = msgType;
	acceptor_.set_option(acceptor_type::reuse_address(true));
	start_accept();
}


TcpServer::TcpServer(unsigned short port, queue_type& q, int msgType, int n):
	  ios_pool_(*boost::factory<io_service_pool*>()(n)),
	  queue_(q),
	  acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	  ,m_session()
{
	m_msgType = msgType;
	acceptor_.set_option(acceptor_type::reuse_address(true));
	start_accept();
}


void TcpServer::start_accept()
{
	m_session.reset(new TcpSession(ios_pool_.get(), queue_, m_msgType));

		acceptor_.async_accept( m_session->socket(), 
			boost::bind(&TcpServer::accept_handler, 
			this, 
			boost::asio::placeholders::error, 
			m_session));

}
// sess不要和session冲突
void TcpServer::accept_handler(const boost::system::error_code& error, TcpSessionPtr session)
{
	if (error)
	{
		gFileLog::instance().Log("TcpServer accept_handler，错误代码:" + boost::lexical_cast<std::string>(error.value()) + "，错误消息:" + error.message());

		session->close();

		// 释放资源
		//delete session;
			
	}
	else
	{
		session->start();
	}

	start_accept();
}


void TcpServer::start()
{
	ios_pool_.start();
}

void TcpServer::run()
{
	ios_pool_.run();
}

void TcpServer::stop()
{
	ios_pool_.stop();
}

