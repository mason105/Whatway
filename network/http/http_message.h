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





class http_message : public IMessage
{
public:


	http_message()
	{
		m_MsgHeaderSize = 4096;
		
		m_MsgHeader.resize(m_MsgHeaderSize);

		m_msgType = MSG_TYPE_HTTP;
	}

	virtual bool DecoderMsgHeader()
	{
		return true;
	}

	
	
};



#endif
