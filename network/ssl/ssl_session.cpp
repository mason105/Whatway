#include "stdafx.h"

#include "ssl_session.h"

#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>


#include "log/FileLogManager.h"

ssl_session::object_pool_type ssl_session::msg_pool_;


ssl_session::ssl_session(ios_type& ios, queue_type& q, boost::asio::ssl::context& context):
	
	socket_(ios, context), 
	strand_(ios), 
	queue_(q)
{
	//socket_.set_verify_mode(boost::asio::ssl::verify_fail_if_no_peer_cert);
	//socket_.set_verify_callback(boost::bind(&ssl_session::verify_certificate, this, _1, _2));
	
}


ssl_session::ios_type& ssl_session::io_service()
{
	return socket_.get_io_service();
}

void ssl_session::close()
{
	OutputDebugString("ssl session close\n");

	boost::system::error_code ignored_ec;

	//socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	//socket_.close(ignored_ec);

	socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	socket_.lowest_layer().close(ignored_ec);
	
}


void ssl_session::start()
{
	socket_.async_handshake(
		boost::asio::ssl::stream_base::server,
        boost::bind(&ssl_session::handle_handshake, shared_from_this(),          boost::asio::placeholders::error)
	);
  
}

void ssl_session::handle_handshake(const boost::system::error_code& error)
{
	if (error)
	{
		//TRACE();
		OutputDebugString("handshake error\n");
		std::string errMsg = error.message();
		close();
		return;

	}

	//ssl_request_ptr req = create_request();
	//read(req);
	read();
}

ssl_request_ptr ssl_session::create_request()
{
	// 内存池
	//return msg_pool_.construct(shared_from_this(), bind(&object_pool_type::destroy, ref(msg_pool_), _1));
	ssl_session_ptr sess = shared_from_this();
	ssl_request_ptr req = boost::factory<ssl_request_ptr>()(sess);
	return req;
}

void ssl_session::read()
{
	ssl_request_ptr req = create_request();

//	boost::asio::async_read(socket_, boost::asio::buffer(RequestMsg.msg_header, tcp_message::header_length),
//		bind(&TcpConnection::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
	// this换成shared_from_this()
	async_read(socket_, 
		boost::asio::buffer(req->msg_header, ssl_message::header_length), 
		strand_.wrap(
			boost::bind(&ssl_session::handle_read_head, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}

void ssl_session::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, ssl_request_ptr req)
{
	if (error) // req->check_head()
	{
		//std::cout << "handle_read_head error" << std::endl;
		//std::cout << "bytes_transferred" << bytes_transferred << std::endl;
		close();
		return;
	}

		//boost::asio::async_read(socket_, boost::asio::buffer(RequestMsg.msg_body, RequestMsg.header.bodysize()), 
		//	boost::bind(&TcpConnection::handle_read_body, shared_from_this(), boost::asio::placeholders::error));
	req->decode_header();

	// this换成shared_from_this()
	async_read(socket_, 
		boost::asio::buffer(req->msg_body, req->header.bodysize()),
		strand_.wrap(
			bind(&ssl_session::handle_read_msg, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, req)
		)
	);
}



void ssl_session::handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, ssl_request_ptr req)
{
	//std::cout << "handle_read_msg bytes_transferred" << bytes_transferred << std::endl;
		
	if (error) // req->check_msg_crc(0
	{
		//std::cout << "handle_read_msg error" << std::endl;

		close();
		return;
	}

	// 得到请求接收时间
	req->genreq();

	queue_.push(req);

	//start();
	// handshake只需一次
	read();
}


void ssl_session::write(ssl_response_ptr resp)
{
	
/*
		boost::asio::async_write(socket_, boost::asio::buffer(ResponseMsg.msg_header, ssl_session::header_length), 
			boost::bind(&TcpConnection::handle_write_header, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

*/
	// this换成shared_from_this()

	async_write(socket_,
		boost::asio::buffer(resp->msg_header, ssl_message::header_length),
		strand_.wrap(
			bind(&ssl_session::handle_write_head, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
}

void ssl_session::handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, ssl_response_ptr resp)
{
	//std::cout << "handle_write_head bytes_transferred " << bytes_transferred << std::endl;

	if (error || bytes_transferred != resp->msg_header.size())
	{
	//	std::cout << "handle_write_head error" << std::endl;
		close();
		return;
	}


	//	boost::asio::async_write(socket_, boost::asio::buffer(ResponseMsg.msg_body, ResponseMsg.msg_body.size()), 
	//		boost::bind(&TcpConnection::handle_write_body, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	// this换成shared_from_this()

	async_write(socket_,
		boost::asio::buffer(resp->msg_body, resp->msg_body.size()),
		strand_.wrap(
			bind(&ssl_session::handle_write_msg, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, resp)
		)
	);
	
}

void ssl_session::handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, ssl_response_ptr resp)
{
	if (error || bytes_transferred != resp->msg_body.size())
	{
		//std::cout << "handle_write_msg error" << std::endl;
		close();
		return;
	}

	boost::posix_time::ptime ptSendTime = boost::posix_time::microsec_clock::local_time();
	std::string sSendTime = boost::gregorian::to_iso_extended_string(ptSendTime.date()) + " " + boost::posix_time::to_simple_string(ptSendTime.time_of_day());
	resp->log.set_sendtime(sSendTime);

	// 存入日志队列
	
	gFileLogManager::instance().push(resp->log);

	resp->destroy();
}

