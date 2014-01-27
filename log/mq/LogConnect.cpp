#include "stdafx.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/format.hpp>

#include "LogConnect.h"
#include "./output/FileLog.h"



LogConnect::LogConnect(void)
	:socket(ios)
	,deadline(ios)
{
	
	m_bConnected = false;

	// 默认连接超时时间
	connectTimeout = 1;

	// 默认读写超时时间
	readWriteTimeout = 5;



	deadline.expires_at(boost::posix_time::pos_infin);
	
	check_deadline();
}

LogConnect::~LogConnect(void)
{
}

// 超时回调函数
void LogConnect::check_deadline()
{
	
	if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
	{
		gFileLog::instance().Log("连接超时或读写超时");

		Close();

		deadline.expires_at(boost::posix_time::pos_infin);
	}
	
	deadline.async_wait( boost::bind(&LogConnect::check_deadline, this) );
}

bool LogConnect::Connect(std::string server)
{
	std::vector<std::string> kv;
	boost::split(kv, server, boost::is_any_of(":")); // 注意需要通过配置文件配置

	m_sIP = kv[0];
	m_nPort = boost::lexical_cast<int>(kv[1]);

	return Connect(m_sIP, m_nPort);
}

// 建立连接
bool LogConnect::Connect(std::string ip, int port)
{
	

	try
	{
		m_bConnected = false;
	
		m_sIP = ip;
		m_nPort = port;

		boost::system::error_code ec;

		boost::asio::ip::tcp::resolver resolver(ios);

		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), m_sIP, boost::lexical_cast<std::string>(m_nPort));

		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, ec);

		// 设置连接超时
		
		deadline.expires_from_now( boost::posix_time::seconds(connectTimeout*1000) );

		ec = boost::asio::error::would_block;

		boost::asio::async_connect(socket, iterator, boost::lambda::var(ec) = boost::lambda::_1);
	
		do 
			ios.run_one(); 
		while (ec == boost::asio::error::would_block);

		if (ec || !socket.is_open())
		{
			std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
			std::string sErrMsg = ec.message();
			std::string sErrInfo = "连接日志服务器失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
			gFileLog::instance().Log(sErrInfo);
			
			
			m_bConnected = false;
			return m_bConnected;
		}

		gFileLog::instance().Log("连接日志服务器成功!");
		m_bConnected = true;
		return m_bConnected;
	}
	catch(std::exception& e)
	{
		gFileLog::instance().Log("连接日志服务器异常：" + std::string(e.what()));
		m_bConnected = false;
		return m_bConnected;
	}
}

bool LogConnect::IsConnected()
{
	
	return m_bConnected;
}



bool LogConnect::Write(CustomMessage * pReq)
{
	if (!WriteMsgHeader(pReq))
		return false;

	if (!WriteMsgContent(pReq))
		return false;

	return true;
}

// 写包头
bool LogConnect::WriteMsgHeader(CustomMessage * pReq)
{
	boost::system::error_code ec = boost::asio::error::would_block;

	
	
	boost::asio::async_write(socket, 
		boost::asio::buffer(pReq->GetMsgHeader(), sizeof(MSG_HEADER)), 
		boost::asio::transfer_all(), 
		boost::lambda::var(ec) = boost::lambda::_1);
	
	do 
		ios.run_one(); 
	while (ec == boost::asio::error::would_block);

	if (ec)
	{
		std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
		std::string sErrMsg = ec.message();
		std::string sErrInfo = "写包头失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

bool LogConnect::WriteMsgContent(CustomMessage * pReq)
{
	boost::system::error_code ec = boost::asio::error::would_block;

	boost::asio::async_write(socket, 
		boost::asio::buffer(pReq->GetMsgContent(), pReq->GetMsgContentSize()), 
		boost::asio::transfer_all(), 
		boost::lambda::var(ec) = boost::lambda::_1);
	
	do 
		ios.run_one(); 
	while (ec == boost::asio::error::would_block);

	if (ec)
	{
		std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
		std::string sErrMsg = ec.message();
		std::string sErrInfo = "写包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

bool LogConnect::Read(CustomMessage * pRes)
{
	if (!ReadMsgHeader(pRes))
		return false;

	if (!ReadMsgContent(pRes))
		return false;

	
	return true;
}

// 读包头
bool LogConnect::ReadMsgHeader(CustomMessage * pRes)
{
	boost::system::error_code ec = boost::asio::error::would_block;

	boost::asio::async_read(socket, 
		boost::asio::buffer(pRes->GetMsgHeader(), sizeof(MSG_HEADER)), 
		boost::asio::transfer_all(), 
		boost::lambda::var(ec) = boost::lambda::_1);
	do 
		ios.run_one(); 
	while (ec == boost::asio::error::would_block);

	if (ec)
	{
		std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
		std::string sErrMsg = ec.message();
		std::string sErrInfo = "读包头失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

// 读包内容
bool LogConnect::ReadMsgContent(CustomMessage * pRes)
{
	boost::system::error_code ec = boost::asio::error::would_block;

	if (!pRes->DecoderMsgHeader())
	{
		return false;
	}
		
	boost::asio::async_read(socket, 
		boost::asio::buffer(pRes->GetMsgContent(), pRes->GetMsgContentSize()),
		boost::asio::transfer_all(), 
		boost::lambda::var(ec) = boost::lambda::_1);
	do 
		ios.run_one(); 
	while (ec == boost::asio::error::would_block);

		
	if (ec)
	{
		std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
		std::string sErrMsg = ec.message();
		std::string sErrInfo = "读包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;			
	}
		
	m_bConnected = true;
	return m_bConnected;
}

// 关闭连接
void LogConnect::Close()
{
	m_bConnected = false;

	boost::system::error_code ec;

	

	socket.close(ec);
	
	if (ec)
	{
		gFileLog::instance().Log("断开日志服务器异常：" + ec.message());
	}

	
	gFileLog::instance().Log("断开日志服务器!");
}

bool LogConnect::ReConnect()
{
	Close();

	return Connect(this->m_sIP, this->m_nPort);
}

// 发送心跳包
bool LogConnect::HeartBeat()
{
	bool bRet = false;

	/*
	if (!m_bConnected)
		return false;

	std::string SOH = "\x01";

	std::string request = "cssweb_funcid=999999" + SOH;

	

	
	
	// 设置读写超时
	//int nReadWriteTimeout = sConfigManager::instance().m_nReadWriteTimeout;
	//deadline.expires_from_now( boost::posix_time::seconds(nReadWriteTimeout) );

	// 发送请求
	CustomMessage * pReq = new CustomMessage();

	

	pReq->SetMsgContent(request);
	pReq->SetMsgHeader(MSG_TYPE_REQUEST, FUNCTION_HEARTBEAT);

	int temp = pReq->GetMsgHeaderSize();

	bRet = Write(pReq);
	delete pReq;
	if (!bRet)
		return false;

	// 接收应答
	CustomMessage * pRes = new CustomMessage();
	bRet = Read(pRes);
	if (bRet)
	{
		//std::string response(pRes->GetPkgBody().begin(),pRes->GetPkgBody().end());
		gFileLog::instance().Log("应答内容：" + pRes->GetMsgContentString());
	}
	else
	{
	}
	delete pRes;	

	*/

	return bRet;
}

void LogConnect::SetConnectTimeout(int connecTimeout)
{
	this->connectTimeout = connectTimeout;
}

void LogConnect::SetReadWriteTimeout(int readWriteTimeout)
{
	this->readWriteTimeout = readWriteTimeout;
}

bool LogConnect::Send(std::string& request, std::string& response)
{
	bool bRet = false;

	/*
	deadline.expires_from_now( boost::posix_time::seconds(readWriteTimeout*1000) );

	// 发送请求
	CustomMessage * pReq = new CustomMessage();

	

	pReq->SetMsgContent(request);
	pReq->SetMsgHeader(MSG_TYPE_REQUEST, FUNCTION_HEARTBEAT);

	int temp = pReq->GetMsgHeaderSize();

	bRet = Write(pReq);
	delete pReq;
	if (!bRet)
		return false;

	// 接收应答
	CustomMessage * pRes = new CustomMessage();
	bRet = Read(pRes);
	if (bRet)
	{
		response = pRes->GetMsgContentString();
		
	}
	else
	{
		// 失败
	}
	delete pRes;	
	*/

	return bRet;
}
