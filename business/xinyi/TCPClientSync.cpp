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
#include "config/ConfigManager.h"


CTCPClientSync::CTCPClientSync(void)
{
	m_bConnected = false;
}

CTCPClientSync::~CTCPClientSync(void)
{
}



// 建立连接
bool CTCPClientSync::CreateConnect()
{
	int rc = 0;
	u_long bio = 1;
	int connectTimeout = m_Counter->m_nConnectTimeout;
	int readTimeout = m_Counter->m_nRecvTimeout * 1000;
	int writeTimeout = m_Counter->m_nRecvTimeout * 1000;
	sockfd = INVALID_SOCKET;


	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET )
	{
		return FALSE;
	}
	
	//设置为非阻塞模式
	bio = 1;
	rc = ioctlsocket(sockfd, FIONBIO, &bio); 
	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_Counter->m_sIP.c_str());
	addr.sin_port = htons(m_Counter->m_nPort);

	rc = connect(sockfd, (const sockaddr *)&addr, sizeof(addr));
	// 异步模式不用判断
	if (rc == SOCKET_ERROR)
	{
		//closesocket(sockfd);
		//return FALSE;
	}

	
	
	
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);

	struct timeval timeout;
	timeout.tv_sec = connectTimeout;
	timeout.tv_usec = 0;

	rc = select(0, NULL, &writefds, NULL, &timeout);
	if (rc == 0)
	{
		// timeout
		closesocket(sockfd);
		return FALSE;
	}

	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}

	if(!FD_ISSET(sockfd, &writefds))  
    {  
		closesocket(sockfd);
		return FALSE;
    }  

	 
	// 设置为阻塞模式
	bio = 0;
	rc = ioctlsocket(sockfd, FIONBIO, &bio);
	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}

	// 设置读写超时
	//rc = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&readTimeout, sizeof(readTimeout));
    //rc = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(char*)&writeTimeout, sizeof(writeTimeout));

	return TRUE;
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
	if (!Send(pReq->GetMsgHeader().data(), sizeof(MSG_HEADER), 0) )
	{

		//std::string sErrInfo = "新意服务器写包头失败，错误代码：" + "sErrCode" + ", 错误消息：" + "sErrMsg";
		std::string sErrInfo = "新意服务器写包头失败";
		gFileLog::instance().Log(sErrInfo);

		m_bConnected = false;
		return m_bConnected;
	}

	m_bConnected = true;
	return m_bConnected;
}

bool CTCPClientSync::WriteMsgContent(CustomMessage * pReq)
{
	if (!Send(pReq->GetMsgContent().data(), pReq->GetMsgContentSize(), 0) )
	{
		
		//std::string sErrInfo = "新意服务器写包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		std::string sErrInfo = "新意服务器写包内容失败";
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
	if (!Recv(pRes->GetMsgHeader().data(), sizeof(MSG_HEADER), 0) )
	{
		
		//std::string sErrInfo = "新意服务器读包头失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		std::string sErrInfo = "新意服务器读包头失败";
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
	

	if (!pRes->DecoderMsgHeader())
	{
		return false;
	}
		

	if (!Recv(pRes->GetMsgContent().data(), pRes->GetMsgContentSize(), 0) )
	{
		
		//std::string sErrInfo = "新意服务器读包内容失败，错误代码：" + sErrCode + ", 错误消息：" + sErrMsg;
		std::string sErrInfo = "新意服务器读包内容失败";
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
	closesocket(sockfd);
	sockfd = INVALID_SOCKET;
	
	m_bConnected = false;
}





bool CTCPClientSync::Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	// 临时修改，无论新意服务器业务处理成功，还是失败，都不让TradeServer处理逻辑

	status = 1;

	bool bRet = true;
	

	// 发送请求
	CustomMessage * pReq = new CustomMessage(MSG_TYPE_TCP_NEW);

	

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
	CustomMessage* pRes = new CustomMessage(MSG_TYPE_TCP_NEW);
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


int CTCPClientSync::Send(const char * buf, int len, int flags)
{
	int rc = 0;
	
	int totalBytes = 0;

	do
	{
		int bytes = 0;
		bytes = send(sockfd, buf + totalBytes, len - totalBytes, flags);
		if (bytes == 0)
		{
			// connection is closed
			break;
		}

		if (bytes < 0)
		{
			break;
		}

		totalBytes += bytes;


	} while(totalBytes < len);

	if (totalBytes != len)
		rc = FALSE;
	else
		rc = TRUE;

	return rc;
}

int CTCPClientSync::Recv(char* buf, int len, int flags)
{
	int rc = 0;
	int totalBytes = 0;

	do
	{
		int bytes = 0;
		bytes = recv(sockfd, buf + totalBytes, len - totalBytes, flags);
		if (bytes == 0)
		{
			// connection is closed
			break;
		}

		if (bytes < 0)
		{
			break;
		}

		totalBytes += bytes;


	} while(totalBytes < len);

	if (totalBytes != len)
		rc = FALSE;
	else
		rc = TRUE;

	return rc;
}
