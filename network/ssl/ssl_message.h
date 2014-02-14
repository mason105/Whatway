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

//#include "ssl_session.h"


#include "./log/tradelog.pb.h"
#include "network/imessage.h"

class ssl_message : public IMessage
{
public:
	quote::PkgHeader pbHeader;

	ssl_message()
	{	
		m_MsgHeaderSize = 23;
		m_MsgHeader.resize(m_MsgHeaderSize);

		msgType = MSG_TYPE_SSL_PB;
	}

	
	

	virtual bool DecoderMsgHeader()
	{
		if (!pbHeader.ParseFromArray(m_MsgHeader.data(), m_MsgHeader.size()))
		{
			return false;
		}

		m_MsgContent.resize(pbHeader.bodysize());
		return true;
	}
	/*
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
	*/

};

#endif
