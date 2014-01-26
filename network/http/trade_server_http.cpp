#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "trade_server_http.h"
#include "common.h"
#include "./config/configmanager.h"


//#include "aes.h"
//#include "base64.h"
//#include "trade.pb.h"
#include "./encrypt/MyBotan.h"

#include "./business/szkingdom/tradebusiness.h"
#include "./business/hundsun_t2/tradebusinesst2.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"

#include "./output/FileLog.h"

trade_server_http::trade_server_http()
:req_worker_(recvq_, boost::bind(&trade_server_http::process_msg, this, _1), gConfigManager::instance().m_nHttpWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&trade_server_http::send_msg, this, _1), gConfigManager::instance().m_nHttpSendThreadPool)
	
{
}

bool trade_server_http::send_msg(http_response_ptr resp)
{
	// 根据http request header判断 是否添加gzip压缩头和功能
	resp->get_session()->write(resp);

	return true;
}

bool trade_server_http::process_msg(http_request_ptr req)
{
	// 分析原始http请求
	std::string url = req->msg_header.data();
	OutputDebugString(url.c_str());

	std::string http_header;
	std::string http_content;
	
	
	std::string response;



	// 处理crossdomain.xml请求
	if (url.find("crossdomain.xml") != std::string::npos)
	{
		crossdomain(req);
		return true;
	}

	// 从url分离出真正的请求串
	//https://jy.njzq.cn/tradegw/?ZnVuY2lkPTMwNDEwMSZjbGllbnRpcD0xODAuMTY4LjEzOC4yMTAmcmFtPTAuNDc5ODQ1NzU4NTI3NTE3MyZjc3N3ZWJfY29kZWtleT0xLjAmRklEX0tISD0xMDEwMTAwNDI2MTMmY3Nzd2ViX2NsaWVudHR5cGU9MiZjc3N3ZWJfdHlwZT1HRVRfU1RPQ0tfUE9TSVRJT04mbWFjPSZGSURfRVhGTEc9MSZpc1NhZmU9MA==
	//GET /tradegw/?ZnVuY2lkPTMwNDEwMSZjbGllbnRpcD0xODAuMTY4LjEzOC4yMTAmcmFtPTAuNDc5ODQ1NzU4NTI3NTE3MyZjc3N3ZWJfY29kZWtleT0xLjAmRklEX0tISD0xMDEwMTAwNDI2MTMmY3Nzd2ViX2NsaWVudHR5cGU9MiZjc3N3ZWJfdHlwZT1HRVRfU1RPQ0tfUE9TSVRJT04mbWFjPSZGSURfRVhGTEc9MSZpc1NhZmU9MA== HTTP/1.1


	// 找不到/tradegw/?上下文
	size_t pos1 = url.find(gConfigManager::instance().m_sFlashUrlContext);
	if (pos1 == std::string::npos)
	{
		invalid_request(req);
		return true;
	}

	size_t pos2 = url.find(" HTTP/1"); // first char is space
	int begin = pos1 + gConfigManager::instance().m_sFlashUrlContext.length();
	int count = pos2 - begin;

	std::string request = url.substr(begin, count);

	//base64
	//ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/还有=号
	// 应该采用urldecode，不是置换，不是一种好的办法
	boost::replace_all(request, "*", "+");


	// 对url请求进行base64解码dec_request
	int outlen;
	char decoder[4096];
	memset(decoder, 0x00, sizeof(decoder));
	
	bool bRet = g_MyBotan.Base64Decoder(request, decoder, &outlen);
	if (!bRet)
	{
		invalid_request(req);
		return true;
	}


	// 查询全部基金
	//dec_request = "funcid=410810&custorgid=3106&custid=3515613&ofcode=&qryflag=1&fundid=68008610&isSafe=0&count=2000&ext=1&poststr=&netaddr=180168138210&cssweb_type=FUND_GET_FUND_INFO&operway=7&ram=0.7543609705753624&tacode=&mac=&orgid=3106&KCBP_PARAM_SERVICENAME=410810&KCBP_PARAM_RESERVED=3106&KCBP_PARAM_PACKETTYPE=2&clientip=180.168.138.210&hardinfo=&trdpwd=Jv010DC/IqD4dNHwpCqC5g$$";

	//查询持仓
	//dec_request = "funcid=410530&custorgid=3106&cssweb_type=GET_FUNDS&netaddr=180168138210&fundid=&ext=1&custid=3515613&hardinfo=111100020655WD-WX51A80R1748     1C659D62A07707/30/2010&operway=7&ram=0.3872804599814117&clientip=180.168.138.210&orgid=3106&isSafe=1&KCBP_PARAM_RESERVED=310608&mac=60:EB:69:5C:7F:92&moneytype=&KCBP_PARAM_PACKETTYPE=2&KCBP_PARAM_SERVICENAME=410530&trdpwd=14df4bed5afb25c6e7b25481a975ab3c";
			
	//查询今日成交
	//dec_request = "funcid=410512&ext=1&netaddr=203156240131&isSafe=1&ram=0.777408595662564&operway=7&hardinfo=1111000206a75WS0H5FE            BC773718178A03/02/2011&poststr=&ordersno=&count=1000&mac=BC:77:37:18:17:8A&fundid=68008610&bankcode=&stkcode=&orgid=3106&KCBP_PARAM_SERVICENAME=410512&market=&secuid=&KCBP_PARAM_PACKETTYPE=2&clientip=203.156.240.131&cssweb_type=GET_TODAY_TRADE&KCBP_PARAM_RESERVED=310608&custorgid=3106&custid=3515613&qryflag=1&trdpwd=14df4bed5afb25c6e7b25481a975ab3c";

	
	std::string dec_request = decoder;

	

	http_response_ptr resp = boost::factory<http_response_ptr>()(req->get_session());

	Trade::TradeLog logMsg;
	std::string recvtime = boost::gregorian::to_iso_extended_string(req->RecvTime.date()) + " " + boost::posix_time::to_simple_string(req->RecvTime.time_of_day());
	logMsg.set_recvtime(recvtime);


	// 处理业务请求


	std::string sysNo;
	std::string busiType;

	GetSysNoAndBusiType(dec_request, sysNo, busiType);

	std::string msg = "系统编号: " + sysNo + "业务类型: " + busiType;
	gFileLog::instance().Log(msg);

	CounterType ct = gConfigManager::instance().GetCounterType(sysNo, busiType);

	if (ct == CT_JZ_WIN)
	{
		TradeBusiness * trade = new TradeBusiness();
		trade->Process(dec_request, http_content, logMsg);
		delete trade;
	}
	else if (ct == CT_HS_T2)
	{
		TradeBusinessT2 * trade = new TradeBusinessT2();
		trade->Process(dec_request, http_content, logMsg);
		delete trade;
	}
	else if (ct == CT_DINGDIAN)
	{
		// 延迟初始化函数指针，只允许初始化一次
		
		g_DingDian.Init();

		TradeBusinessDingDian * trade = new TradeBusinessDingDian();
		//trade->Process(request_gbk, response, logMsg);
		trade->Process(dec_request, http_content, logMsg);
		delete trade;
	}
	else
	{
		std::string SOH = "\x01";

		http_content = "1";
		http_content += SOH;
		http_content += "3";
		http_content += SOH;

		http_content += "cssweb_code";
		http_content += SOH;
		http_content += "cssweb_msg";
		http_content += SOH;
		http_content += "cssweb_gwInfo";
		http_content += SOH;

		http_content += "1001";
		http_content += SOH;
		http_content += "自定义参数，没有找到对应的柜台类型";
		http_content += SOH;
		http_content += "0.0.0.0:5000";
		http_content += SOH;
	
		gFileLog::instance().Log("flash 没有找到对应的柜台类型");
	}


	// 对http_content进行base64编码
	std::string encoder;
	//bool Base64Encoder(const unsigned char* src, int len, std::string& encoder);
	//AfxMessageBox(http_content.c_str());
	bRet = g_MyBotan.Base64Encoder((const unsigned char*)http_content.c_str(), http_content.length(), encoder);
	//char * out = Base64Encode(0,http_content.c_str(), http_content.length());
	//AfxMessageBox(encoder.c_str());
	if (!bRet)
	{
		invalid_request(req);
		return true;
	}


	http_content = encoder;
	// 置换操作,这一步没什么必要
	//boost::replace_all(http_content, "+", "*");

	
	// 生成响应内容response
	response = "HTTP/1.1 200 OK\r\n"; // HTTP 1.1
	response += "Content-Type: application/text\r\n";
	response += "Content-Length: ";
	std::string sHttpContentLen = boost::lexical_cast<std::string>(http_content.length());
	response += sHttpContentLen;
	response += "\r\n\r\n";
	response += http_content;
	// Connection:Close


	// 发送响应包response到队列
	resp->logMsg = logMsg;

	int msglen = response.length();
	resp->set_body_size(msglen);
	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	sendq_.push(resp);
	

	// 释放请求包
	req->destroy();


	return true;
}



void trade_server_http::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void trade_server_http::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

trade_server_http::req_queue_type& trade_server_http::recv_queue()
{
	return recvq_;
}


void trade_server_http::log()
{
	//log_.push();
}

void trade_server_http::crossdomain(http_request_ptr req)
{
	std::string response;

	response = "HTTP/1.1 200 OK\r\n"; // HTTP 1.1
	response += "Content-Type: application/xml\r\n";
	response += "Content-Length: ";
	std::string sHttpContentLen = boost::lexical_cast<std::string>(gConfigManager::instance().m_sFlashCrossDomain.length());
	response += sHttpContentLen;
	response += "\r\n\r\n";
	response += gConfigManager::instance().m_sFlashCrossDomain;


	http_response_ptr resp = boost::factory<http_response_ptr>()(req->get_session());
	
	int msglen = response.length();

	resp->set_body_size(msglen);
	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	
	sendq_.push(resp);

	// 释放请求包
	req->destroy();
}

void trade_server_http::invalid_request(http_request_ptr req)
{
	std::string response;

	response = "HTTP/1.1 400 BadRquest\r\n";
	response += "Content-Length: 0\r\n";
	response += "\r\n";

	http_response_ptr resp = boost::factory<http_response_ptr>()(req->get_session());
	
	int msglen = response.length();

	resp->set_body_size(msglen);
	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	
	sendq_.push(resp);

	// 释放请求包
	req->destroy();
}

bool trade_server_http::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType)
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

	}

	if (sysNo.empty() || busiType.empty())
		return false;

	return true;
}
