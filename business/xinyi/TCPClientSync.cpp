#include "stdafx.h"

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

#include "TCPClientSync.h"
#include "./output/FileLog.h"

#include "ConnectPool/counter.h"


CTCPClientSync::CTCPClientSync(void)
	:socket(ios)
	,deadline(ios)
{
	
	m_bConnected = false;


	deadline.expires_at(boost::posix_time::pos_infin);
	
	check_deadline();
}

CTCPClientSync::~CTCPClientSync(void)
{
}

// 超时回调函数
void CTCPClientSync::check_deadline()
{
	
	if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
	{
		gFileLog::instance().Log("新意服务器：连接超时或读写超时");

		CloseConnect();

		deadline.expires_at(boost::posix_time::pos_infin);
	}
	
	deadline.async_wait( boost::bind(&CTCPClientSync::check_deadline, this) );
}

// 建立连接
bool CTCPClientSync::CreateConnect()
{
	try
	{
		m_bConnected = false;
	
		m_sIP = m_Counter->m_sIP;
		m_nPort = m_Counter->m_nPort;

		boost::system::error_code ec;

		boost::asio::ip::tcp::resolver resolver(ios);

		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), m_sIP, boost::lexical_cast<std::string>(m_nPort));

		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, ec);

		// 设置连接超时
		//int nConnectTimeout = sConfigManager::instance().m_nConnectTimeout;
		//deadline.expires_from_now( boost::posix_time::seconds(nConnectTimeout) );

		ec = boost::asio::error::would_block;

		boost::asio::async_connect(socket, iterator, boost::lambda::var(ec) = boost::lambda::_1);
	
		do 
			ios.run_one(); 
		while (ec == boost::asio::error::would_block);

		if (ec || !socket.is_open())
		{
			std::string sErrCode = boost::lexical_cast<std::string>(ec.value());
			std::string sErrMsg = ec.message();
			std::string sErrInfo = "连接新意服务器失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
			gFileLog::instance().Log(sErrInfo);
			
			
			m_bConnected = false;
			return m_bConnected;
		}

		gFileLog::instance().Log("连接新意服务器成功!");
		m_bConnected = true;
		return m_bConnected;
	}
	catch(std::exception& e)
	{
		gFileLog::instance().Log("连接新意服务器异常：" + std::string(e.what()));
		m_bConnected = false;
		return m_bConnected;
	}
}



bool CTCPClientSync::Write(CustomMessage * pReq)
{
	if (!WriteMsgHeader(pReq))
		return false;

	if (!WriteMsgContent(pReq))
		return false;

	return true;
}

// 写包头
bool CTCPClientSync::WriteMsgHeader(CustomMessage * pReq)
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
		std::string sErrInfo = "新意服务器写包头失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

bool CTCPClientSync::WriteMsgContent(CustomMessage * pReq)
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
		std::string sErrInfo = "新意服务器写包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

bool CTCPClientSync::Read(CustomMessage * pRes)
{
	if (!ReadMsgHeader(pRes))
		return false;

	if (!ReadMsgContent(pRes))
		return false;

	
	return true;
}

// 读包头
bool CTCPClientSync::ReadMsgHeader(CustomMessage * pRes)
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
		std::string sErrInfo = "新意服务器读包头失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

// 读包内容
bool CTCPClientSync::ReadMsgContent(CustomMessage * pRes)
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
		std::string sErrInfo = "新意服务器读包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;			
	}
		
	m_bConnected = true;
	return m_bConnected;
}

// 关闭连接
void CTCPClientSync::CloseConnect()
{
	m_bConnected = false;

	boost::system::error_code ec;

	

	socket.close(ec);
	
	if (ec)
	{
		gFileLog::instance().Log("断开新意服务器异常：" + ec.message());
	}

	
	gFileLog::instance().Log("断开新意服务器!");
}



void CTCPClientSync::SetConnectTimeout(int connecTimeout)
{
	this->connectTimeout = connectTimeout;
}

void CTCPClientSync::SetReadWriteTimeout(int readWriteTimeout)
{
	this->readWriteTimeout = readWriteTimeout;
}

bool CTCPClientSync::Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	bool bRet = true;
	

	// 发送请求
	CustomMessage * pReq = new CustomMessage();

	

	pReq->SetMsgContent(request);

	MSG_HEADER binRespMsgHeader;
	binRespMsgHeader.CRC = 0;
	binRespMsgHeader.FunctionNo = 1;
	binRespMsgHeader.MsgContentSize = request.size();
	binRespMsgHeader.MsgType = MSG_TYPE_REQUEST;
	binRespMsgHeader.zip = 0;
	memcpy(&(pReq->m_MsgHeader.front()), &binRespMsgHeader, sizeof(MSG_HEADER));

	//pReq->SetMsgHeader();

	int temp = pReq->GetMsgHeaderSize();

	bRet = Write(pReq);
	delete pReq;
	if (!bRet)
		return false;

	// 接收应答
	CustomMessage* pRes = new CustomMessage();
	bRet = Read(pRes);
	if (bRet)
	{
		response = pRes->GetMsgContentString();
		//std::string response(pRes->GetPkgBody().begin(),pRes->GetPkgBody().end());
		gFileLog::instance().Log("新意服务器应答内容：" + pRes->GetMsgContentString());
	}
	else
	{
	}
	delete pRes;	
	
	return bRet;
}

// 发送心跳包
bool CTCPClientSync::HeartBeat()
{
	bool bRet = false;
	/*
	if (!m_bConnected)
		return false;

	std::string SOH = "\x01";

	std::string request = "cssweb_funcid=999999" + SOH;

	

	// 开始时间
	 boost::posix_time::ptime time_sent = boost::posix_time::microsec_clock::universal_time();
	
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

	// 结束时间
	boost::posix_time::ptime time_received = boost::posix_time::microsec_clock::universal_time();

	// 运行时间
	int nRuntime = (time_received - time_sent).total_microseconds();
	//gFileLog::instance().Log("执行时间：" + boost::lexical_cast<std::string>(nRuntime));
	*/
	return bRet;
}