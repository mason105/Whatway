#include "stdafx.h"

#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>

#include "TcpSession.h"



#include "./output/FileLog.h"

#include "log/FileLogManager.h"
#include "log/DistributedLogManager.h"



#include "network/imessage.h"
#include "network/http/http_message.h"
#include "network/tcp/tcp_message_old.h"
#include "network/ssl/ssl_message.h"
#include "network/ssl_tcp/custommessage.h" 


TcpSession::TcpSession( ios_type& ios, queue_type& q, int msgType):
	socket_(ios), 
	strand_(ios), 
	queue_(q)
{
	
	m_msgType = msgType;
}

TcpSession::~TcpSession()
{
	
}


TcpSession::socket_type& TcpSession::socket()
{
	return socket_;
}

TcpSession::ios_type& TcpSession::io_service()
{
	return socket_.get_io_service();
}

void TcpSession::close()
{
	// 关闭柜台连接
	CloseCounterConnect();

	boost::system::error_code ignored_ec;

	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

	socket_.close(ignored_ec);

	// 释放Server创建的指针
	delete this;
}


void TcpSession::start()
{
	read();
}

IMessage* TcpSession::create_request()
{
	IMessage* req = NULL;

	switch (m_msgType)
	{
	case MSG_TYPE_HTTP:
		req = new http_message();
		break;
	case MSG_TYPE_TCP_OLD:
		req = new tcp_message_old();
		break;
	case MSG_TYPE_SSL_PB:
		req = new ssl_message();
		break;
	case MSG_TYPE_TCP_NEW:
		req = new CustomMessage();
		break;
	}

	req->SetSession(this);
	return req;

	// 内存池
	//return msg_pool_.construct(shared_from_this(), bind(&object_pool_type::destroy, ref(msg_pool_), _1));

	//tcp_session_old_ptr sess = shared_from_this();
	//tcp_old_request_ptr req = boost::factory<tcp_old_request_ptr>()(shared_from_this());
	//tcp_old_request_ptr req = new tcp_old_request((tcp_session_old_ptr)shared_from_this());
	
}


// 读请求
void TcpSession::read()
{
	IMessage* req = create_request();	

	boost::asio::async_read(socket_, 
		boost::asio::buffer(req->GetMsgHeader(), req->GetMsgHeaderSize()), 
		boost::asio::transfer_all(),
		strand_.wrap(
			boost::bind(&TcpSession::handle_read_head, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}

void TcpSession::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req)
{
	if (error)
	{
		gFileLog::instance().Log("TcpSession 读包头失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != req->GetMsgHeaderSize())
	{
		gFileLog::instance().Log("TcpSession 读包头失败，需要读:" + boost::lexical_cast<std::string>(req->GetMsgHeaderSize()) + ", 实际读:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	
	if (!req->DecoderMsgHeader())
	{
		gFileLog::instance().Log("TcpSession 解码包头失败");

		close();
		return;
	}

	async_read(socket_, 
		boost::asio::buffer(req->GetMsgContent(), req->GetMsgContentSize()),
		boost::asio::transfer_all(),
		strand_.wrap(
			bind(&TcpSession::handle_read_msg, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}



void TcpSession::handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req)
{
	
		
	if (error) 
	{
	
		gFileLog::instance().Log("TcpSession 读包内容失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != req->GetMsgContentSize())
	{
		gFileLog::instance().Log("TcpSession 读包内容失败 需要读:" + boost::lexical_cast<std::string>(req->GetMsgContentSize()) + ", 实际读:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	
	// 设置接收到的时间
	req->SetRecvTime();

	queue_.push(req);

	read();
}

// 写应答数据
void TcpSession::write(IMessage* resp)
{
	boost::asio::async_write(socket_,
		boost::asio::buffer(resp->GetMsgHeader(), resp->GetMsgHeaderSize()),
		boost::asio::transfer_all(),
		strand_.wrap(
			bind(&TcpSession::handle_write_head, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
}

void TcpSession::handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp)
{
	if (error)
	{
		gFileLog::instance().Log("TcpSession 写包头失败，错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}


	if (bytes_transferred != resp->GetMsgHeaderSize())
	{
		gFileLog::instance().Log("TcpSession 写包头失败 需要写:" + boost::lexical_cast<std::string>(resp->GetMsgHeaderSize()) + ", 实际写:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	
	boost::asio::async_write(socket_,
		boost::asio::buffer(resp->GetMsgContent(), resp->GetMsgContentSize()),
		boost::asio::transfer_all(),
		strand_.wrap(
			bind(&TcpSession::handle_write_msg, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
	
}

void TcpSession::handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp)
{

	if (error)
	{

		gFileLog::instance().Log("TcpSession 写包内容失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != resp->GetMsgContentSize())
	{
		gFileLog::instance().Log("TcpSession 写包内容失败 需要写:" + boost::lexical_cast<std::string>(resp->GetMsgContentSize()) + ", 实际写:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	// 存入日志队列
	resp->SetSendTime();

	gFileLogManager::instance().push(resp->log);
	gDistributedLogManager::instance().push(resp->log);

	/*
	if (msgHeader.FunctionNo == 0)
	{
		// 心跳功能不写日志
	}
	else
	{
		
	}
	*/

	// 删除应答包
	resp->destroy();
}

