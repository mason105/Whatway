#ifndef _TCP_MESSAGE_OLD_
#define _TCP_MESSAGE_OLD_



#include <vector>
#include <string>

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

class tcp_message_old : public IMessage
{
public:
	tcp_message_old()
	{
		m_MsgHeaderSize = 4;
		
		m_MsgHeader.resize(m_MsgHeaderSize);

		msgType = MSG_TYPE_TCP_OLD;
	}

	
	virtual bool DecoderMsgHeader()
	{
		int msgContentSize = 0 ;
		memcpy(&msgContentSize, m_MsgHeader.data(), m_MsgHeader.size());

		msgContentSize = ntohl(msgContentSize);
		if (msgContentSize <= 0)
		{
			return false;
		}

		m_MsgContent.resize(msgContentSize);
		return true;
		
	}
};


#endif
