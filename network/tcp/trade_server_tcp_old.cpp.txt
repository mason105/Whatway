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
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include "trade_server_tcp_old.h"

#include "common.h"
#include "./config/configmanager.h"


#include "./business/szkingdom_win/tradebusiness.h"
#include "./business/hundsun_t2/tradebusinesst2.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"



#include "./output/FileLog.h"



trade_server_tcp_old::trade_server_tcp_old()
	:req_worker_(recvq_, boost::bind(&trade_server_tcp_old::process_msg, this, _1), gConfigManager::instance().m_nTcpWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&trade_server_tcp_old::send_msg, this, _1), gConfigManager::instance().m_nTcpSendThreadPool)

{
}

bool trade_server_tcp_old::send_msg(IMessage* resp)
{
	resp->GetSession()->write(resp);
	return true;
}



void trade_server_tcp_old::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void trade_server_tcp_old::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

trade_server_tcp_old::req_queue_type& trade_server_tcp_old::recv_queue()
{
	return recvq_;
}


bool trade_server_tcp_old::process_msg(IMessage* req)
{
	/*
	//int size = req->msg_body.size();
	//std::string request(req->msg_body.begin(), req->msg_body.end());

	std::string request = req->request;

	std::string sysNo;
	std::string busiType;
	bool bIsHeartBeat = false;

	GetSysNoAndBusiType(request, sysNo, busiType, bIsHeartBeat);

	//std::string msg = "系统编号: " + sysNo + "业务类型: " + busiType;
	//gFileLog::instance().Log(msg);

//	std::vector<char>::iterator it;
//	for ( it=req->msg_body.begin() ; it != req->msg_body.end(); it++ )
//	{
//		request += *it;
//	}


  //std::copy(req->msg_body.begin(), req->msg_body.end(), std::ostream_iterator<std::string>(sstr));
  //std::string request = sstr.str();
	//std::string request;
	//request.assign(req->msg_body.begin(), req->msg_body.end());


	//std::copy( req->msg_body.begin(), req->msg_body.end(), std::back_inserter(request));

	std::string response;

	Trade::TradeLog logMsg;
	
	// 请求包中的接收时间
	std::string recvtime = boost::gregorian::to_iso_extended_string(req->RecvTime.date()) + " " + boost::posix_time::to_simple_string(req->RecvTime.time_of_day());
	logMsg.set_recvtime(recvtime);

	tcp_old_response_ptr resp = boost::factory<tcp_old_response_ptr>()(req->get_session());

	

	if (bIsHeartBeat)
	{
		// 用于日志处理根据功能号过滤心跳功能
		logMsg.set_funcid("999999");
		//logMsg.request = request;
		//Sleep(10*1000);只是用于客户端读写超时

			std::string SOH = "\x01";

			response = "1";
			response += SOH;
			response += "3";
			response += SOH;

			response += "cssweb_code";
			response += SOH;
			response += "cssweb_msg";
			response += SOH;
			response += "cssweb_gwInfo";
			response += SOH;

			response += "999999";
			response += SOH;
			response += "心跳检测";
			response += SOH;
			response += "0.0.0.0:5000";
			response += SOH;
	}
	else
	{
		CounterType ct = gConfigManager::instance().GetCounterType(sysNo, busiType);
		int status = 0;
	std::string errCode = "";
	std::string errMsg = "";
		if (ct ==  CT_JZ_WIN)
		{
			TradeBusiness * trade = new TradeBusiness();
			trade->Send(request, response, status, errCode, errMsg);
			delete trade;
		}
		else if (ct ==  CT_HS_T2)
		{
			TradeBusinessT2 * trade = new TradeBusinessT2();
			trade->Send(request, response, status, errCode, errMsg);
			delete trade;
		}
		else if (ct == CT_DINGDIAN)
		{
			g_DingDian.Init();

			TradeBusinessDingDian * trade = new TradeBusinessDingDian();
			trade->Send(request, response, status, errCode, errMsg);
			delete trade;
		}
		else if (ct == CT_JSD)
		{
			//TradeBusinessAGC * trade = new TradeBusinessAGC();
			//trade->Process(request, response, logMsg);
			//delete trade;
		}
		else if (ct == CT_XINYI)
		{
//			TradeBusinessXinyi * trade = new TradeBusinessXinyi();
//			trade->Process(request, response, logMsg);
//			delete trade;
		}
		else
		{
			std::string SOH = "\x01";

			response = "1";
			response += SOH;
			response += "3";
			response += SOH;

			response += "cssweb_code";
			response += SOH;
			response += "cssweb_msg";
			response += SOH;
			response += "cssweb_gwInfo";
			response += SOH;

			response += "1001";
			response += SOH;
			response += "自定义参数，没有找到对应的柜台类型";
			response += SOH;
			response += "0.0.0.0:5000";
			response += SOH;
	
			gFileLog::instance().Log("tcp 没有找到对应的柜台类型");
		}
	}


	//产生响应

	//tcp_old_response_ptr resp = new tcp_message_old(req->get_session());
	int msglen = response.length();
	response.size();
	resp->set_body_size(msglen);

	// 业务处理日志
	resp->log = logMsg;

//	std::string tmp = boost::lexical_cast<std::string>(msglen);
//	OutputDebugString("reponse length:");
//	OutputDebugString(tmp.c_str());
//	OutputDebugString("\n");

	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	resp->encode_header();
	sendq_.push(resp);

	// 释放请求
	req->destroy();
	*/
	return true;
}

bool trade_server_tcp_old::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType, bool& bIsHeartBeat)
{
	

	std::vector<std::string> keyvalues;
	std::string SOH = "\x01";
	boost::split(keyvalues, request, boost::is_any_of(SOH)); // 注意需要通过配置文件配置

	// 全部转成小写
	//取的时候也全部改成小写

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

		if (key == "cssweb_funcid" && value == "999999")
			bIsHeartBeat = true;

	}

	if (sysNo.empty() || busiType.empty())
		return false;

	return true;
}
