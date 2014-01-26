#ifndef _TCP_MESSAGE_OLD_
#define _TCP_MESSAGE_OLD_

#include "./network/pkgheader.pb.h"
#include "./network/communication.pb.h"
//#include "tcp_session.h"

#include <vector>
#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <winsock2.h>

#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "./log/tradelog.pb.h"


class tcp_session_old;
typedef boost::shared_ptr<tcp_session_old> tcp_session_old_ptr;

class tcp_message_old : boost::noncopyable
{
public:
	typedef boost::function<void(tcp_message_old*)> destroy_type;
	
	

private:
	tcp_session_old_ptr session_;
	destroy_type destroy_;

public:
	template<typename Func>
	tcp_message_old(tcp_session_old_ptr& s, Func func):
		session_(s), destroy_(func)
	{
	}

	tcp_message_old(tcp_session_old_ptr& s):
		session_(s)
	{
	}

	tcp_message_old()
	{
	}

	//tcp_session_old_ptr get_session()
	tcp_session_old_ptr& get_session()
	{
		return session_;
	}

	void destroy()
	{
		if (destroy_)
		{
			//std::cout << "tcp_message_old destroy" << std::endl;
			destroy_(this);
		}
		else
		{
			OutputDebugString("==================tcp_message_old checked_delete===================\n");
			boost::checked_delete(this);
			//delete this;
		}
	}
	
	enum {header_length = 4};

	// 消息头
	typedef boost::array<char, header_length> msg_header_type; // char[]
	msg_header_type msg_header;
	

	bool decode_header()
	{
		//body_size_ = msg_header.data();
		memcpy(&body_size_, msg_header.data(), msg_header.size());

		body_size_ = ntohl(body_size_);
		if (body_size_ <= 0)
		{
			OutputDebugString("decode_header error\n");
			return false;
		}

		set_body_size(body_size_);


		return true;
	}

	bool encode_header()
	{
		int tmp;
		tmp = htonl(body_size_);
/*
会是负数，不能加这个判断条件
		if (tmp < 0)
		{
			OutputDebugString("encode_header error\n");
			return false;
		}
*/

		memcpy(&(msg_header.front()), &tmp, 4);
		return true;
	}

	// 消息体
	std::vector<char> msg_body;
	int body_size_;
	

	void set_body_size(int body_size)
	{
		msg_body.clear();
		msg_body.resize(body_size);

		body_size_ = body_size;
	}


	std::string request;
	void genreq()
	{
		request.assign(msg_body.begin(), msg_body.end());

		 RecvTime = boost::posix_time::microsec_clock::local_time();
	}

	// 接收到请求的时间
	boost::posix_time::ptime RecvTime;
	
	// 业务处理生成的日志
	Trade::TradeLog logMsg;

	// 发送响应的时间
	boost::posix_time::ptime SendTime;
};

typedef tcp_message_old tcp_old_request;
typedef tcp_message_old tcp_old_response;
typedef tcp_message_old* tcp_old_request_ptr;
typedef tcp_message_old* tcp_old_response_ptr;

#endif
