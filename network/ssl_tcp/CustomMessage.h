#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#include <vector>

#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "MsgHeader.h"
//#include "tcpsession.h"

#include "network/imessage.h"

/*
独立出来的原因，
1.以后可以改由内存池分配
2.异步调用的时候适合放入队列
*/




class CustomMessage : public IMessage
{
public:
	CustomMessage(int msgType);

private:
	// 消息头
	struct MsgHeader msgHeader;
	

	

	
	

public:
	
	

	
	
	bool DecoderMsgHeader();


	/*
	PMSG_HEADER GetMsgHeader();
	


	
	char * GetMsgContent();
	
	

	
	

	
	

	*/

	
};



#endif