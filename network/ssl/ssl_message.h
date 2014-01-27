#ifndef _SSL_MESSAGE_
#define _SSL_MESSAGE_



#include <vector>

#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "./network/ssl/pkgheader.pb.h"
#include "./network/ssl/communication.pb.h"
//#include "ssl_session.h"


#include "./log/tradelog.pb.h"
#include "network/imessage.h"

class ssl_session;
typedef boost::shared_ptr<ssl_session> ssl_session_ptr;

class ssl_message : public IMessage
{
public:
	typedef boost::function<void(ssl_message*)> destroy_type;
	
	

private:
	ssl_session_ptr session_;
	destroy_type destroy_;

public:
	template<typename Func>
	ssl_message(const ssl_session_ptr s, Func func):
		session_(s), destroy_(func)
	{
	}

	ssl_message(const ssl_session_ptr s):
		session_(s)
	{
	}

	ssl_session_ptr get_session()
	{
		return session_;
	}

	void destroy()
	{
		if (destroy_)
		{
			//std::cout << "ssl_message destroy" << std::endl;
			destroy_(this);
		}
		else
		{
			
			boost::checked_delete(this);
		}
	}
	
	enum {header_length = 23};

	// 消息头
	typedef boost::array<char, header_length> msg_header_type; // char[]
	msg_header_type msg_header;
	quote::PkgHeader header;

	bool decode_header()
	{
		// 把头序列化为pb格式
		
		//std::cout << "msg_header size " << msg_header.size() << std::endl;

		if (!header.ParseFromArray(msg_header.data(), msg_header.size()))
		{
			//std::cout << "序列化消息头失败\n";
					return false;
		}
//std::cout << "序列化消息头成功\n";
		//body_size_ = header.bodysize();
	set_body_size(header.bodysize());
		//std::cout << "消息体大小是 " << header.bodysize() << std::endl;

			//int req_pkgbody_length = req_pkgheader_.bodysize();
			//char * req_pkgbody = new char[req_pkgbody_length + 1];

		
		return true;
	}

	// 消息体
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

	bool encode_header(quote::PkgHeader::MsgType msgType, int errcode, bool zip=0)
	{
		header.set_bodysize(body_size_);
		header.set_ver(1);
		header.set_enc(0);
		header.set_zip(zip);
		header.set_more(0);
		header.set_msgtype(msgType);
		header.set_errcode(errcode);
		
		if (!header.SerializeToArray(&msg_header, header.ByteSize()))
			return false;

		return true;
	}

	void genreq()
	{
		//request.assign(msg_body.begin(), msg_body.end());

		 RecvTime = boost::posix_time::microsec_clock::local_time();
	}

	// 接收到请求的时间
	boost::posix_time::ptime RecvTime;
	
	

	// 发送响应的时间
	boost::posix_time::ptime SendTime;
};

typedef ssl_message ssl_request;
typedef ssl_message ssl_response;
typedef ssl_message* ssl_request_ptr;
typedef ssl_message* ssl_response_ptr;

#endif
