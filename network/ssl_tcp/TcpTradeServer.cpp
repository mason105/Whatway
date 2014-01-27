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

#include "TcpTradeServer.h"

#include "common.h"
#include "./config/configmanager.h"


#include "./business/szkingdom/tradebusiness.h"
#include "./business/hundsun_t2/tradebusinesst2.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"



#include "./output/FileLog.h"

#include "connectpool/connectmanager.h"

// 新意
#include "business/xinyi/TCPClientSync.h"

// AGC
#include "business/SunGuard/SywgConnect.h"


#include "log/FileLogManager.h"
#include "errcode.h"

#include "network/ssl_tcp/TcpSession.h"
#include "network/ssl_tcp/SSLSession.h"


TcpTradeServer::TcpTradeServer()
	:req_worker_(recvq_, boost::bind(&TcpTradeServer::ProcessRequest, this, _1), gConfigManager::instance().m_nTcpWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&TcpTradeServer::ProcessResponse, this, _1), gConfigManager::instance().m_nTcpSendThreadPool)
	
{
}





void TcpTradeServer::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void TcpTradeServer::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

TcpTradeServer::req_queue_type& TcpTradeServer::recv_queue()
{
	return recvq_;
}

// 处理应答
bool TcpTradeServer::ProcessResponse(CustomMessage* resp)
{
	resp->GetSession()->write(resp);
	return true;
}

// 处理请求，由于是线程函数，不要使用共享数据
bool TcpTradeServer::ProcessRequest(CustomMessage* req)
{
	std::string SOH = "\x01";

	Trade::TradeLog::LogLevel logLevel = Trade::TradeLog::INFO_LEVEL;

	std::string sysNo = "";
	std::string busiType = "";
	int nBusiType;
	std::string sysVer = "";
	std::string funcId = "";
	int nFuncId = req->GetMsgHeader()->FunctionNo;
	std::string account = "";
	std::string clientIp = "";

	std::string gatewayIp = "";
	std::string gatewayPort = "";
	std::string counterIp = "";
	std::string counterPort = "";
	std::string counterType = "";

	
	boost::posix_time::ptime ptBeginTime;
	std::string beginTime = "";
	int runtime = 0;


	std::string request = req->GetMsgContentString();
	std::string response = "";
	int status = 0;
	std::string errCode = "";
	std::string errMsg = "";


	

	// 如果消息类型不是请求类型
	if (req->GetMsgHeader()->MsgType != MSG_TYPE_REQUEST)
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

	// 初始化柜台连接
	if (req->GetSession()->counterConnect == NULL)
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
				
		switch(counter->m_eCounterType)
		{
		case CT_HS_T2:
			break;
		case CT_HS_COM:
			break;
		case CT_JZ_WIN:
			break;
		case CT_JZ_LINUX:
			break;
		case CT_DINGDIAN:
			break;
		case CT_XINYI:
			{
			req->GetSession()->counterConnect = new CTCPClientSync();
			req->GetSession()->counterConnect->SetCounterServer(counter);
			counterIp = counter->m_sIP;
			counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
			counterType = GetCounterType(counter->m_eCounterType);
			break;
			}
		case CT_JSD:
			{
			req->GetSession()->counterConnect = new CSywgConnect();
			req->GetSession()->counterConnect->SetCounterServer(counter);
			counterIp = counter->m_sIP;
			counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
			counterType = GetCounterType(counter->m_eCounterType);
			break;
			}
		default:
			break;
		}
	}
	else
	{
		counterIp = req->GetSession()->counterConnect->counter->m_sIP;
		counterPort = boost::lexical_cast<std::string>(req->GetSession()->counterConnect->counter->m_nPort);
		counterType = GetCounterType(req->GetSession()->counterConnect->counter->m_eCounterType);
	}

	
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
			if (req->GetSession()->counterConnect->IsConnected())
			{
				// 已建立连接，跳出循环
				bConnect = true;
				break;
			}
			else
			{
				ptBeginTime = boost::posix_time::microsec_clock::local_time();
				beginTime = boost::gregorian::to_iso_extended_string(ptBeginTime.date()) + " " + boost::posix_time::to_simple_string(ptBeginTime.time_of_day());;

				if (req->GetSession()->counterConnect->Connect())
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

					Counter * counter = NULL;
					counter = g_ConnectManager.GetServer(sysNo, gConfigManager::instance().ConvertIntToBusiType(nBusiType), "0000");
					req->GetSession()->counterConnect->SetCounterServer(counter);

					counterIp = counter->m_sIP;
					counterPort = boost::lexical_cast<std::string>(counter->m_nPort);
					counterType = GetCounterType(counter->m_eCounterType);
					
					logLevel = Trade::TradeLog::ERROR_LEVEL;

					errCode = boost::lexical_cast<std::string>(CONNECT_COUNTER_ERROR);
					errMsg = gError::instance().GetErrMsg(CONNECT_COUNTER_ERROR);

					boost::posix_time::ptime ptEndTime = boost::posix_time::microsec_clock::local_time();
					runtime = (ptEndTime - ptBeginTime).total_microseconds();// 微秒数

					req->Log(Trade::TradeLog::ERROR_LEVEL, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);
					gFileLogManager::instance().push(req->log);
				}
			}
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
		
		// 处理业务，业务失败或成功都算成功的，只有通信失败才需要重试
		if (req->GetSession()->counterConnect->Send(request, response, status, errCode, errMsg))
		{
			logLevel = Trade::TradeLog::INFO_LEVEL;

			boost::posix_time::ptime ptEndTime = boost::posix_time::microsec_clock::local_time();
			runtime = (ptEndTime - ptBeginTime).total_microseconds();// 微秒数

			//req->Log(Trade::TradeLog::, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);

			break;
		}
		else
		{
			// 通信失败,开始重试
			// 处理一次，写一次日志
			// fileLog.push(req->log)
			//req->GetSession()->counterConnect->counter->m_eCounterType
			
			
		}
	} // end for retry
	



finish:

	CustomMessage * resp = new CustomMessage();

	req->Log(logLevel, sysNo, sysVer, busiType, funcId, account, clientIp, request, response, status, errCode, errMsg, beginTime, runtime, gatewayIp, gatewayPort, counterIp, counterPort, counterType);
	resp->log = req->log; // 拷贝日志消息


	// 设置会话
	resp->SetSession(req->GetSession());

	
	
	// 设置消息内容
	resp->SetMsgContent(response);

	// 设置消息头
	resp->SetMsgHeader(MSG_TYPE_RESPONSE_END, nFuncId);

	
	

	// 释放请求
	req->destroy();

	sendq_.push(resp);

	
	return true;
}

bool TcpTradeServer::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType, std::string& sysVer, std::string& account, std::string& funcId, std::string& clientIp)
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

std::string TcpTradeServer::GetCounterType(COUNTER_TYPE counterType)
{
	switch (counterType)
	{
	case CT_HS_T2:
		return "1";
	case CT_HS_COM:
		return "2";
	case CT_JZ_WIN:
		return "3";
	case CT_JZ_LINUX:
		return "4";
	case CT_DINGDIAN:
		return "5";
	case CT_JSD:
		return "6";
	case CT_XINYI:
		return "7";
	default:
		return "0";
	}
}