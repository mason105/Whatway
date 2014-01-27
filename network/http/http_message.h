#ifndef _HTTP_MESSAGE_
#define _HTTP_MESSAGE_


#include <vector>

#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "./log/tradelog.pb.h"
#include "network/imessage.h"


class http_session;
typedef boost::shared_ptr<http_session> http_session_ptr;

class http_message : public IMessage
{
public:
	typedef boost::function<void(http_message*)> destroy_type;
	
	

private:
	http_session_ptr session_;
	destroy_type destroy_;

public:
	template<typename Func>
	http_message(const http_session_ptr s, Func func):
		session_(s), destroy_(func)
	{
	}

	http_message(const http_session_ptr s):
		session_(s)
	{
		memset(msg_header.data(), 0x00, msg_header.size());
	}

	http_session_ptr get_session()
	{
		return session_;
	}

	void destroy()
	{
		if (destroy_)
		{
//			std::cout << "http_message destroy" << std::endl;
			destroy_(this);
		}
		else
		{
//			std::cout << "http_message checked_delete" << std::endl;
			boost::checked_delete(this);
		}
	}
	
	enum {header_length = 4096};


	typedef boost::array<char, header_length> msg_header_type; // char[]
	msg_header_type msg_header;

	// 消息体
	// 可以存放文本和二进制数据
	std::vector<char> msg_body;
	//boost::shared_ptr<std::vector<char>> body;
	//boost::scoped_array<char> body; // new char[]
	int body_size_;

	void set_body_size(int body_size)
	{
		msg_body.clear();
		msg_body.resize(body_size);

		body_size_ = body_size;
	}

	// 接收到请求的时间
	boost::posix_time::ptime RecvTime;
	
	

	// 发送响应的时间
	boost::posix_time::ptime SendTime;

	void genreq()
	{
		//request.assign(msg_body.begin(), msg_body.end());

		 RecvTime = boost::posix_time::microsec_clock::local_time();
	}
};

typedef http_message http_request;
typedef http_message http_response;
typedef http_message* http_request_ptr;
typedef http_message* http_response_ptr;

#endif
