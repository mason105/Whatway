#ifndef _ISESSION_H_
#define _ISESSION_H_

#include <boost/asio.hpp>

#include "AbstractBusiness.h"
//#include "network/CustomMessage.h"

class CustomMessage;

class ISession
{
public:
	ISession(void);
	~ISession(void);

	AbstractBusiness * counterConnect;

	virtual void write(CustomMessage* resp)=0;

	virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* req)=0;
	virtual void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* req)=0;

	virtual void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* resp)=0;
	virtual void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, CustomMessage* resp)=0;
};

#endif
