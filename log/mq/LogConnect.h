#ifndef _MQ_CONNECT_SYNC_
#define _MQ_CONNECT_SYNC_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/scoped_array.hpp> 
#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/system/system_error.hpp>

#include "Network/ssl_tcp/CustomMessage.h"
#include "ConnectPool/Counter.h"

class LogConnect
{
public:
	LogConnect();
	~LogConnect(void);

	bool Connect(std::string ip, int port);
	bool Connect(std::string server);
	void Close();

	bool ReConnect();
	bool IsConnected();

	// 心跳功能，由于后台业务不同，所以封装
	bool HeartBeat();

	virtual bool Send(std::string& request, std::string& response);

	void SetConnectTimeout(int seconds);
	void SetReadWriteTimeout(int seconds);
	

private:
	bool Write(CustomMessage * pReq);
	bool WriteMsgHeader(CustomMessage * pReq);
	bool WriteMsgContent(CustomMessage * pReq);

	bool Read(CustomMessage * pRes);
	bool ReadMsgHeader(CustomMessage * pRes);
	bool ReadMsgContent(CustomMessage * pRes);

	void check_deadline();
	std::string decompress(std::string data);

private:
	boost::asio::io_service ios;
	boost::asio::ip::tcp::socket socket;
	boost::asio::deadline_timer deadline;
	

	std::string m_sIP;
	int m_nPort;

	bool m_bConnected;

	int connectTimeout;
	int readWriteTimeout;


	std::string logFileName;

	
};

#endif
