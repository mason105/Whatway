#include "stdafx.h"
//#define BOOST_FILESYSTEM_VERSION 2
// mongodb用的还是老版本，只支持2，未来会自带boost1.49

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/functional/factory.hpp>
//压缩
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>


#include "TradeServer.h"

#include "common.h"
#include "./config/configmanager.h"

// 金证
#include "./business/szkingdom_win/tradebusiness.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
// 恒生
#include "./business/hundsun_t2/tradebusinesst2.h"
// 顶点
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"
// AGC
#include "business/SunGuard/SywgConnect.h"
// 新意
#include "business/xinyi/TCPClientSync.h"




#include "./output/FileLog.h"

#include "connectpool/connectmanager.h"




#include "log/FileLogManager.h"
#include "errcode.h"

#include "network/ssl_tcp/TcpSession.h"
#include "network/ssl_tcp/SSLSession.h"

#include "network/http/http_message.h"
#include "network/tcp/tcp_message_old.h"
#include "network/ssl/ssl_message.h"
#include "network/ssl_tcp/custommessage.h"


TradeServer::TradeServer(int msgType)
	:req_worker_(recvq_, boost::bind(&TradeServer::ProcessRequest, this, _1), gConfigManager::instance().m_nTcpWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&TradeServer::ProcessResponse, this, _1), gConfigManager::instance().m_nTcpSendThreadPool)
	
{
	m_MsgType = msgType;
}





void TradeServer::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void TradeServer::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

TradeServer::req_queue_type& TradeServer::recv_queue()
{
	return recvq_;
}

// 处理应答
bool TradeServer::ProcessResponse(IMessage* resp)
{
	
	resp->GetSession()->write(resp);

	return true;
}

// 处理请求，由于是线程函数，不要使用共享数据
bool TradeServer::ProcessRequest(IMessage* req)
{
	std::string SOH = "\x01";

	Trade::TradeLog::LogLevel logLevel = Trade::TradeLog::INFO_LEVEL;

//	MSG_HEADER binMsgHeader;
//	memcpy(&binMsgHeader, req->GetMsgHeader().data(), req->GetMsgHeaderSize());

	std::string sysNo = "";
	std::string busiType = "";
	int nBusiType;
	std::string sysVer = "";
	std::string funcId = "";
	int nFuncId = -1;
	std::string account = "";
	std::string clientIp = "";

	std::string gatewayIp = "";
	std::string gatewayPort = "";
	std::string gatewayServer = "";

	std::string counterIp = "";
	std::string counterPort = "";
	std::string counterType = "";
	int nCounterType;
	std::string counterServer = "";

	
	boost::posix_time::ptime ptBeginTime;
	std::string beginTime = "";
	int runtime = 0;


	std::string request = req->GetMsgContentString();
	std::string response = "";
	int status = 0;
	std::string errCode = "";
	std::string errMsg = "";

	if (m_MsgType == MSG_TYPE_TCP_OLD)
	{
		gatewayIp = gConfigManager::instance().m_sIp;

		gatewayPort = boost::lexical_cast<std::string>(gConfigManager::instance().m_nTcpPort);
	}
	if (m_MsgType == MSG_TYPE_SSL_PB)
	{
		gatewayIp = gConfigManager::instance().m_sIp;

		gatewayPort = boost::lexical_cast<std::string>(gConfigManager::instance().m_nSslPort);
	}
	if (m_MsgType == MSG_TYPE_TCP_NEW)
	{
		gatewayIp = gConfigManager::instance().m_sIp;

		gatewayPort = boost::lexical_cast<std::string>(gConfigManager::instance().m_nTcpNewPort);
	}
	if (m_MsgType == MSG_TYPE_SSL_NEW)
	{
		gatewayIp = gConfigManager::instance().m_sIp;

		gatewayPort = boost::lexical_cast<std::string>(gConfigManager::instance().m_nSslNewPort);
	}
	gatewayServer = gatewayIp + ":" + gatewayPort;
			
							
			
			
	/*

	// 如果消息类型不是请求类型
	if (binMsgHeader.MsgType != MSG_TYPE_REQUEST)
	{
		logLevel = Trade::TradeLog::ERROR_LEVEL;

		errCode = boost::lexical_cast<std::string>(MSG_HEADER_ERROR);
		errMsg = gError::instance().GetErrMsg(MSG_HEADER_ERROR);

		response = "1" + SOH + "2" + SOH;
		response += "cssweb_code";
		response += SOH;
		response += "cssweb_msg";
		response += SOH;
		response += errCode;
		response += SOH;
		response += errMsg;
		response += SOH;

		
		goto finish;
	}
	*/

	// 客户端心跳功能
	if (nFuncId == FUNCTION_HEARTBEAT)
	{
		response = "heartbeat";

		goto finish;
	}

	

	
	if (!GetSysNoAndBusiType(request, sysNo, busiType, sysVer, account, funcId, clientIp))
	{
		logLevel = Trade::TradeLog::ERROR_LEVEL;

		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);

		response = "1" + SOH + "2" + SOH;
		response += "cssweb_code";
		response += SOH;
		response += "cssweb_msg";
		response += SOH;
		response += errCode;
		response += SOH;
		response += errMsg;
		response += SOH;

		goto finish;
	}

	nBusiType = boost::lexical_cast<int>(busiType);

	// 得到柜台类型
	nCounterType = g_ConnectManager.GetCounterType(sysNo, busiType);
	if (nCounterType == COUNTER_TYPE_UNKNOWN)
	{
	}

	/*
	// 初始化柜台连接
	if (req->GetSession()->GetCounterConnect() == NULL)
	{
		Counter * counter = NULL;
		counter = g_ConnectManager.GetServer(sysNo, gConfigManager::instance().ConvertIntToBusiType(nBusiType), "0000");
		if (counter == NULL)
		{
			logLevel = Trade::TradeLog::ERROR_LEVEL;

			errCode = boost::lexical_cast<std::string>(CONFIG_ERROR);
			errMsg = gError::instance().GetErrMsg(CONFIG_ERROR);

			response = "1" + SOH + "2" + SOH;
			response += "cssweb_code";
			response += SOH;
			response += "cssweb_msg";
			response += SOH;
			response += errCode;
			response += SOH;
			response += errMsg;
			response += SOH;


			goto finish;
		}
				
		

		req->GetSession()->GetCounterConnect()->SetCounterServer(counter);

		counterIp = counter->m_sIP;
		counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
		counterType = GetCounterType(counter->m_eCounterType);
	}
	else
	{
		counterIp = req->GetSession()->GetCounterConnect(nCounterType)->m_Counter->m_sIP;
		counterPort = boost::lexical_cast<std::string>(req->GetSession()->GetCounterConnect(nCounterType)->m_Counter->m_nPort);
		counterType = GetCounterType(req->GetSession()->GetCounterConnect(nCounterType)->m_Counter->m_eCounterType);
	}
	*/
	
	int serverCount = g_ConnectManager.GetServerCount(sysNo, gConfigManager::instance().ConvertIntToBusiType(nBusiType), "0000");
	if (serverCount == 0)
	{
	}



	// 连接需要处理负载均衡和故障切换
	// 业务重试还需要考虑吗？
	/*
	* 如果没有连接，建立连接，处理请求
	* 如果建立连接失败，轮询每一个服务器，如果所有服务器连接失败，返回错误
	* 当前连接失效，send调用失败，重试
	*/


	
	
	// 业务重试
	ptBeginTime = boost::posix_time::microsec_clock::local_time();
	beginTime = boost::gregorian::to_iso_extended_string(ptBeginTime.date()) + " " + boost::posix_time::to_simple_string(ptBeginTime.time_of_day());;

	for (int retry=0; retry<1; retry++)
	{
		bool bConnect = false;
	
		// 轮询每一个服务器
		for (int i=0; i<serverCount; i++)
		{
			if (req->GetSession()->GetCounterConnect(nCounterType)->IsConnected())
			{
				

				// 已建立连接，跳出循环
				bConnect = true;
				break;
			}
			else
			{
				ptBeginTime = boost::posix_time::microsec_clock::local_time();
				beginTime = boost::gregorian::to_iso_extended_string(ptBeginTime.date()) + " " + boost::posix_time::to_simple_string(ptBeginTime.time_of_day());;

				Counter * counter = NULL;
				counter = g_ConnectManager.GetServer(sysNo, gConfigManager::instance().ConvertIntToBusiType(nBusiType), "0000");
				counterIp = counter->m_sIP;
				counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
				counterType = GetCounterType(counter->m_eCounterType);
				
				req->GetSession()->GetCounterConnect(nCounterType)->SetCounterServer(counter);

				if (req->GetSession()->GetCounterConnect(nCounterType)->CreateConnect())
				{
					// 建立连接成功，跳出循环
					bConnect = true;
					break;
				}
				else
				{
					// 建立连接失败
					// 轮询算法：返回下一个服务器
					bConnect = false;

					//Counter * counter = NULL;
					//counter = g_ConnectManager.GetServer(sysNo, gConfigManager::instance().ConvertIntToBusiType(nBusiType), "0000");
					//req->GetSession()->GetCounterConnect(nCounterType)->SetCounterServer(counter);

					
					
					logLevel = Trade::TradeLog::ERROR_LEVEL;

					errCode = boost::lexical_cast<std::string>(CONNECT_COUNTER_ERROR);
					errMsg = gError::instance().GetErrMsg(CONNECT_COUNTER_ERROR);

					boost::posix_time::ptime ptEndTime = boost::posix_time::microsec_clock::local_time();
					runtime = (ptEndTime - ptBeginTime).total_microseconds();// 微秒数

					req->Log(Trade::TradeLog::ERROR_LEVEL, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);
					gFileLogManager::instance().push(req->log);
				} // end if
			} // end if
		} // end for (int i=0; i<serverCount; i++)

		// 所有服务器连接不上
		if (!bConnect)
		{
			logLevel = Trade::TradeLog::ERROR_LEVEL;

			errCode = boost::lexical_cast<std::string>(CONNECT_ALL_COUNTER_ERROR);
			errMsg = gError::instance().GetErrMsg(CONNECT_ALL_COUNTER_ERROR);

			response = "1" + SOH + "2" + SOH;
			response += "cssweb_code";
			response += SOH;
			response += "cssweb_msg";
			response += SOH;
			response += errCode;
			response += SOH;
			response += errMsg;
			response += SOH;

			
			goto finish;
		}

		// 每次请求都需要记录柜台的ip和port
				Counter * counter = req->GetSession()->GetCounterConnect(nCounterType)->m_Counter;
				counterIp = counter->m_sIP;
				counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
				counterType = GetCounterType(counter->m_eCounterType);
				counterServer = counterIp + ":"+ counterPort;
		
		// 处理业务，业务失败或成功都算成功的，只有通信失败才需要重试
		if (req->GetSession()->GetCounterConnect(nCounterType)->Send(request, response, status, errCode, errMsg))
		{
			// 业务失败，重构response
			if (status == 0)
			{
				response = "1";
				response += SOH;
				response += "4";
				response += SOH;

				response += "cssweb_code";
				response += SOH;
				response += "cssweb_msg";
				response += SOH;
				response += "cssweb_gwInfo";
				response += SOH;
				response += "cssweb_counter";
				response += SOH;

				response += errCode;
				response += SOH;
				response += errMsg;
				response += SOH;
				response += gatewayServer;
				response += SOH;
				response += counterServer;
				response += SOH;
			}

			logLevel = Trade::TradeLog::INFO_LEVEL;

			boost::posix_time::ptime ptEndTime = boost::posix_time::microsec_clock::local_time();
			runtime = (ptEndTime - ptBeginTime).total_microseconds();// 微秒数



			break;
		}
		else
		{
			continue;
			// 通信失败,开始重试
			// 处理一次，写一次日志
			// fileLog.push(req->log)
			//req->Log(Trade::TradeLog::, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);
		}// end if
	} // end for retry
	



finish:

	std::vector<char> compressedMsgContent;
	

	

	IMessage * resp = NULL;
	
	//std::vector<char> msgHeader;

	switch(req->msgType)
	{
	case MSG_TYPE_HTTP:
		{
			
		resp = new http_message();
		// 设置消息内容
		resp->SetMsgContent(response);

		}
		break;
	case MSG_TYPE_TCP_OLD:
		{
		resp = new tcp_message_old();
		int msgHeaderSize = response.size();
		msgHeaderSize = htonl(msgHeaderSize);
		memcpy(&(resp->m_MsgHeader.front()), &msgHeaderSize, 4);
		// 设置消息内容
		resp->SetMsgContent(response);

		}
		break;
	case MSG_TYPE_SSL_PB:
		{
		resp = new ssl_message();

		quote::PkgHeader pbHeader;

		if (response.size() > gConfigManager::instance().zlib)
		{
			boost::iostreams::filtering_streambuf<boost::iostreams::output> compress_out;
			compress_out.push(boost::iostreams::zlib_compressor());
			compress_out.push(boost::iostreams::back_inserter(compressedMsgContent));
			boost::iostreams::copy(boost::make_iterator_range(response), compress_out);

			pbHeader.set_zip(true);
			int compressedMsgContentSize = compressedMsgContent.size();
			pbHeader.set_bodysize(compressedMsgContentSize);
			// 设置消息内容
			resp->SetMsgContent(compressedMsgContent);

			
		}
		else
		{
			pbHeader.set_zip(false);
			pbHeader.set_bodysize(response.size());
			// 设置消息内容
			resp->SetMsgContent(response);

		}
		
		pbHeader.set_ver(1);
		pbHeader.set_enc(false);
		
		pbHeader.set_more(false);
		pbHeader.set_msgtype(quote::PkgHeader::RES_TRADE);
		pbHeader.set_errcode(0);
		
		
		bool bTmp = pbHeader.SerializeToArray(&(resp->m_MsgHeader.front()), pbHeader.ByteSize());
			
		}
		break;
	case MSG_TYPE_TCP_NEW:
		{
		resp = new CustomMessage();

		MSG_HEADER binRespMsgHeader;
		binRespMsgHeader.CRC = 0;
		binRespMsgHeader.FunctionNo = nFuncId;
		
		binRespMsgHeader.MsgType = MSG_TYPE_RESPONSE_END;

		if (response.size() > gConfigManager::instance().zlib)
		{
			boost::iostreams::filtering_streambuf<boost::iostreams::output> compress_out;
			compress_out.push(boost::iostreams::zlib_compressor());
			compress_out.push(boost::iostreams::back_inserter(compressedMsgContent));
			boost::iostreams::copy(boost::make_iterator_range(response), compress_out);

			binRespMsgHeader.zip = 1;
			int compressedMsgContentSize = compressedMsgContent.size();
			binRespMsgHeader.MsgContentSize = compressedMsgContentSize;
			// 设置消息内容
			resp->SetMsgContent(compressedMsgContent);

		}
		else
		{
			binRespMsgHeader.zip = 0;
			binRespMsgHeader.MsgContentSize = response.size();
			// 设置消息内容
			resp->SetMsgContent(response);

		}

		//msgHeader.resize(sizeof(MSG_HEADER));

		memcpy(&(resp->m_MsgHeader.front()), &binRespMsgHeader, sizeof(MSG_HEADER));
		}
		break;
	}

	// 拷贝日志消息
	req->Log(logLevel, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);
	resp->log = req->log; 


	// 设置会话
	resp->SetSession(req->GetSession());

	
	

	

	
		

	// 设置消息头
	//resp->SetMsgHeader(msgHeader);

	
	

	// 释放请求
	req->destroy();

	sendq_.push(resp);

	
	return true;
}

bool TradeServer::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType, std::string& sysVer, std::string& account, std::string& funcId, std::string& clientIp)
{
	
	std::string SOH = "\x01";

	std::vector<std::string> keyvalues;
	boost::split(keyvalues, request, boost::is_any_of(SOH));


	for (std::vector<std::string>::iterator it = keyvalues.begin(); it != keyvalues.end(); it++)
	{
		std::string keyvalue = *it;


		if (keyvalue.empty())
			break;

		std::vector<std::string> kv;
		boost::split(kv, keyvalue, boost::is_any_of("="));

		std::string key = "";
		if (!kv[0].empty())
			key = kv[0];

		std::string value = "";
		if (!kv[1].empty())
			value = kv[1];


		if (key == "cssweb_sysNo")
			sysNo = value;

		if (key == "cssweb_busiType")
			busiType = value;

		if (key == "cssweb_sysVer")
		{
			sysVer = value;
		}

		if (key == "cssweb_funcid")
		{
			funcId = value;
		}
		
		if (key == "cssweb_account")
		{
			account = value;
		}

	}

	if (sysNo.empty() || busiType.empty())
		return false;

	return true;
}

std::string TradeServer::GetCounterType(int counterType)
{
	switch (counterType)
	{
	case COUNTER_TYPE_HS_T2:
		return "1";
	case COUNTER_TYPE_HS_COM:
		return "2";
	case COUNTER_TYPE_JZ_WIN:
		return "3";
	case COUNTER_TYPE_JZ_LINUX:
		return "4";
	case COUNTER_TYPE_DINGDIAN:
		return "5";
	case COUNTER_TYPE_JSD:
		return "6";
	case COUNTER_TYPE_XINYI:
		return "7";
	default:
		return "0";
	}
}