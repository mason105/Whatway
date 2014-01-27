#ifndef _ISESSION_H_
#define _ISESSION_H_

#include <boost/asio.hpp>

#include "business/IBusiness.h"
//#include "network/CustomMessage.h"

class IMessage;

class ISession
{
public:
	ISession(void);
	~ISession(void);

	// ¹ñÌ¨Á¬½Ó
	IBusiness * counterConnect;

	

//	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req)=0;
//	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req)=0;

	virtual void write(IMessage* resp)=0;
//	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp)=0;
//	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp)=0;
};

#endif
