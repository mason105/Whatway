#include "stdafx.h"

#include "tcp_session_old.h"
#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>
//#include <windows.h>
#include "./output/FileLog.h"
#include "log/FileLogManager.h"


tcp_session_old::object_pool_type tcp_session_old::msg_pool_;


tcp_session_old::tcp_session_old(ios_type& ios, queue_type& q):

	socket_(ios), 
	strand_(ios), 
	queue_(q)
{
}



tcp_session_old::socket_type& tcp_session_old::socket()
{
	return socket_;
}

tcp_session_old::ios_type& tcp_session_old::io_service()
{
	return socket_.get_io_service();
}

void tcp_session_old::close()
{
	OutputDebugString("=================session close=================\n");

	boost::system::error_code ignored_ec;
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	socket_.close(ignored_ec);
}


void tcp_session_old::start()
{
	//tcp_old_request_ptr req = create_request();
	tcp_old_request_ptr req = boost::factory<tcp_old_request_ptr>()(shared_from_this());
	//tcp_old_request_ptr req = new tcp_old_request(shared_from_this());

	read(req);
}

tcp_old_request_ptr tcp_session_old::create_request()
{
	// 内存池
	//return msg_pool_.construct(shared_from_this(), bind(&object_pool_type::destroy, ref(msg_pool_), _1));

	//tcp_session_old_ptr sess = shared_from_this();
	tcp_old_request_ptr req = boost::factory<tcp_old_request_ptr>()(shared_from_this());
	//tcp_old_request_ptr req = new tcp_old_request((tcp_session_old_ptr)shared_from_this());
	return req;
}

void tcp_session_old::read(tcp_old_request_ptr req)
{
	
//	boost::asio::async_read(socket_, boost::asio::buffer(RequestMsg.msg_header, tcp_message::header_length),
//		bind(&TcpConnection::handle_read_header, shared_from_this(), boost::asio::placeholders::error));

	boost::asio::async_read(socket_, 
		boost::asio::buffer(req->msg_header, tcp_message_old::header_length), 
		boost::asio::transfer_all(),
		strand_.wrap(
			boost::bind(&tcp_session_old::handle_read_head, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}

void tcp_session_old::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, tcp_old_request_ptr req)
{
	

	if (error) // req->check_head()
	{
		gFileLog::instance().Log("读包头失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != req->msg_header.size())
	{
		gFileLog::instance().Log("读包头失败，需要读:" + boost::lexical_cast<std::string>(req->msg_header.size()) + ", 实际读:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

		//boost::asio::async_read(socket_, boost::asio::buffer(RequestMsg.msg_body, RequestMsg.header.bodysize()), 
		//	boost::bind(&TcpConnection::handle_read_body, shared_from_this(), boost::asio::placeholders::error));
	if (!req->decode_header())
	{
		gFileLog::instance().Log("解码包头失败");

		close();
		return;
	}

	async_read(socket_, 
		boost::asio::buffer(req->msg_body, req->body_size_),
		boost::asio::transfer_all(),
		strand_.wrap(
			bind(&tcp_session_old::handle_read_msg, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}



void tcp_session_old::handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, tcp_old_request_ptr req)
{
	
		
	if (error) // req->check_msg_crc(0
	{
		//std::cout << "handle_read_msg error" << std::endl;
		gFileLog::instance().Log("读包内容失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != req->msg_body.size())
	{
		gFileLog::instance().Log("读包内容失败 需要读:" + boost::lexical_cast<std::string>(req->msg_body.size()) + ", 实际读:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	req->genreq();

	queue_.push(req);

	start();
}


void tcp_session_old::write(tcp_old_response_ptr resp)
{

	
/*
		boost::asio::async_write(socket_, boost::asio::buffer(ResponseMsg.msg_header, tcp_message::header_length), 
			boost::bind(&TcpConnection::handle_write_header, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

*/
	boost::asio::async_write(socket_,
		boost::asio::buffer(resp->msg_header, tcp_message_old::header_length),
		boost::asio::transfer_all(),
		//boost::asio::transfer_at_least(tcp_message_old::header_length),
		strand_.wrap(
			bind(&tcp_session_old::handle_write_head, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
}

void tcp_session_old::handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, tcp_old_response_ptr resp)
{
	if (error)
	{
		gFileLog::instance().Log("写包头失败，错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}


	if (bytes_transferred != resp->msg_header.size())
	{
		gFileLog::instance().Log("写包头失败 需要写:" + boost::lexical_cast<std::string>(resp->msg_header.size()) + ", 实际写:" + boost::lexical_cast<std::string>(bytes_transferred) );

		

		close();
		return;
	}

	int nPkgBodySize = resp->msg_body.size();
	

	//	boost::asio::async_write(socket_, boost::asio::buffer(ResponseMsg.msg_body, ResponseMsg.msg_body.size()), 
	//		boost::bind(&TcpConnection::handle_write_body, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	boost::asio::async_write(socket_,
		boost::asio::buffer(resp->msg_body, nPkgBodySize),
		boost::asio::transfer_all(),
		strand_.wrap(
			bind(&tcp_session_old::handle_write_msg, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
	
}

void tcp_session_old::handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, tcp_old_response_ptr resp)
{

	if (error)
	{

		gFileLog::instance().Log("写包内容失败， 错误代码:" + boost::lexical_cast<std::string>(error.value()) + ", 错误消息:" + error.message());

		close();
		return;
	}

	if (bytes_transferred != resp->msg_body.size())
	{
		gFileLog::instance().Log("写包内容失败 需要写:" + boost::lexical_cast<std::string>(resp->msg_body.size()) + ", 实际写:" + boost::lexical_cast<std::string>(bytes_transferred) );

		close();
		return;
	}

	// 存入日志队列
	boost::posix_time::ptime ptSendTime = boost::posix_time::microsec_clock::local_time();
	std::string sSendTime = boost::gregorian::to_iso_extended_string(ptSendTime.date()) + " " + boost::posix_time::to_simple_string(ptSendTime.time_of_day());
	resp->logMsg.set_sendtime(sSendTime);

	gFileLogManager::instance().push(resp->logMsg);

	// 删除响应包
	resp->destroy();
}

