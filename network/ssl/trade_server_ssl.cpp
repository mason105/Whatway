#include "stdafx.h"
//#define BOOST_FILESYSTEM_VERSION 2
// mongodb用的还是老版本，只支持2，未来会自带boost1.49

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

//压缩
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
//uuid
#include <boost/uuid/uuid.hpp>  
#include <boost/uuid/uuid_generators.hpp>  
#include <boost/uuid/uuid_io.hpp>  


#include "trade_server_ssl.h"
#include "common.h"



#include "./config/configmanager.h"



//#include <mongo/client/dbclient.h>
//#include <mongo/client/connpool.h>



//#include "trade.pb.h"




#include "./encrypt/mybotan.h"
#include <Magick++.h>
#include "./captcha/captchamanager.h"


#include <unicode/putil.h>
#include <unicode/ucnv.h>

#include "./business/szkingdom_win/tradebusiness.h"
#include "./business/hundsun_t2/tradebusinesst2.h"
#include "./business/hundsun_com/TradeBusinessComm.h"
#include "./business/apexsoft/TradeBusinessDingDian.h"
#include "./business/apexsoft/DingDian.h"



#include "./output/FileLog.h"



trade_server_ssl::trade_server_ssl():
req_worker_(recvq_, boost::bind(&trade_server_ssl::process_msg, this, _1), gConfigManager::instance().m_nSslWorkerThreadPool)
	,resp_worker_(sendq_, boost::bind(&trade_server_ssl::send_msg, this, _1), gConfigManager::instance().m_nSslSendThreadPool)
	
{
}

bool trade_server_ssl::send_msg(IMessage* resp)
{
	// 压缩

	if (resp->GetMsgContentSize() <= gConfigManager::instance().zlib)
	{
		//不压缩
	}
	else
	{
		// 压缩
		//gFileLog::instance().Log("开始压缩");

		std::vector<char> compressed;
		boost::iostreams::filtering_streambuf<boost::iostreams::output> compress_out;
		compress_out.push(boost::iostreams::zlib_compressor());
		compress_out.push(boost::iostreams::back_inserter(compressed));
		boost::iostreams::copy(boost::make_iterator_range(resp->GetMsgContent()), compress_out);

		int bodysize = compressed.size();
		//resp->set_body_size(bodysize);
		//memcpy(&(resp->msg_body.front()), &compressed[0], compressed.size());

		//resp->encode_header(resp->header.msgtype(), resp->header.errcode(), 1);

		//gFileLog::instance().Log("结束压缩");
	}

	resp->GetSession()->write(resp);

	return true;
}




void trade_server_ssl::start()
{
	req_worker_.start();
	resp_worker_.start();
}

void trade_server_ssl::stop()
{
	req_worker_.stop();
	resp_worker_.stop();
}

trade_server_ssl::req_queue_type& trade_server_ssl::recv_queue()
{
	return recvq_;
}

bool trade_server_ssl::process_msg(IMessage* req)
{
	/*
	switch(req->header.msgtype())
	{
	case quote::PkgHeader::REQ_TRADE:
		trade(req);
		break;
	case quote::PkgHeader::REQ_CAPTCHA:
		captcha(req);
		break;
	case quote::PkgHeader::REQ_OCR:
		OCR(req);
		break;

	default:
		error(req);
		break;
	}
	
	req->destroy();
	*/
	return true;
}

void trade_server_ssl::trade(IMessage* req)
{
	/*
	
	int msgtype = req->header.msgtype();
	if (msgtype != quote::PkgHeader::REQ_TRADE )
	{
		return;
//		resp->set_body_size(0);
//		resp->encode_header(quote::PkgHeader::RES_TRADE, EC_REQ_INVALID);
//		sendq_.push(resp);


	}
	//ssl_response_ptr resp = new ssl_message(req->get_session());
	ssl_response_ptr resp = boost::factory<ssl_response_ptr>()(req->get_session());




	std::string request_utf8(req->msg_body.begin(), req->msg_body.end());

//	std::string temp="原始请求:";
//	temp += request_utf8;
//	gFileLog::instance().Log(temp);

	std::string response;
	
	UErrorCode errcode = U_ZERO_ERROR;
	char dest[8192];
	memset(dest, 0x00, sizeof(dest));
	int ret = ucnv_convert("gbk", "utf8", dest, sizeof(dest), request_utf8.c_str(), -1, &errcode);
	std::string request_gbk = dest;

	Trade::TradeLog logMsg;
	// 请求包中的接收时间
	std::string recvtime = boost::gregorian::to_iso_extended_string(req->RecvTime.date()) + " " + boost::posix_time::to_simple_string(req->RecvTime.time_of_day());
	logMsg.set_recvtime(recvtime);

	std::string sysNo;
	std::string busiType;

	GetSysNoAndBusiType(request_gbk, sysNo, busiType);

	std::string msg = "系统编号: " + sysNo + "业务类型: " + busiType;
	gFileLog::instance().Log(msg);

	CounterType ct = gConfigManager::instance().GetCounterType(sysNo, busiType);

	if (ct == CT_JZ_WIN)
	{
		TradeBusiness * trade = new TradeBusiness();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_HS_T2)
	{
		TradeBusinessT2 * trade = new TradeBusinessT2();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_DINGDIAN)
	{
		// 延迟初始化函数指针，只允许初始化一次
		
		g_DingDian.Init();

		TradeBusinessDingDian * trade = new TradeBusinessDingDian();
		trade->Process(request_gbk, response, logMsg);
		delete trade;
	}
	else if (ct == CT_JSD)
	{
//		TradeBusinessAGC * trade = new TradeBusinessAGC();
//		trade->Process(request_gbk, response, logMsg);
//		delete trade;
	}
	else if (ct == CT_XINYI)
	{
//		TradeBusinessXinyi * trade = new TradeBusinessXinyi();
//		trade->Process(request_gbk, response, logMsg);
//		delete trade;
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
	
		gFileLog::instance().Log("ssl 没有找到对应的柜台类型");
	}

	// 业务处理日志
	resp->log = logMsg;

	int msglen = response.length();
	resp->set_body_size(msglen);
	memcpy(&(resp->msg_body.front()), response.c_str(), response.length());
	resp->encode_header(quote::PkgHeader::RES_TRADE, EC_SUCCESS);
	sendq_.push(resp);
	*/
}

void trade_server_ssl::captcha(IMessage* req)
{
//	int msgtype = req->header.msgtype();
//	if (msgtype != quote::PkgHeader::REQ_CAPTCHA)
//	{
//		return;
//	}

	IMessage* resp;// = boost::factory<ssl_message*>()(req->GetSession());

	std::string text = captcha::instance().GetCaptcha();
	int captcha = boost::lexical_cast<int>(text);
	OutputDebugString("captcha ");
	OutputDebugString(text.c_str());
	OutputDebugString("\n");
/*
	std::string backGround = "xc:#CCCCCC";
	std::string buttonSize = "118x41";
	// Button background texture
	std::string buttonTexture = "granite:";
	// Button text
	// Button text color
	std::string textColor = "red";
	// Font point size
	int fontPointSize = 24;
	Magick::Image captcha;
	// Set button size
	captcha.size( buttonSize);
	// Read background image
	captcha.read( backGround);
	// Set background to buttonTexture
	Magick::Image backgroundTexture( buttonTexture );
	captcha.texture( backgroundTexture );
	// Add some text
	captcha.fillColor( textColor );
	captcha.fontPointsize( fontPointSize );
	captcha.annotate( text, Magick::CenterGravity );
	// Write to BLOB in JPEG format
	Magick::Blob blob;
	captcha.magick("JPEG"); // Set JPEG output format
	captcha.write( &blob ); 
*/


		std::string jpeg_b64;

		std::string image_jpeg;
		std::string image_type = "jpg";
		m_Captcha.VerificationCodeToImage(text,image_jpeg, image_type);

		//g_MyBotan.Base64Encoder((const unsigned char*)blob.data(), blob.length(), jpeg_b64);

		g_MyBotan.Base64Encoder((const unsigned char*)image_jpeg.c_str(), image_jpeg.length(), jpeg_b64);
		
		int msglen = jpeg_b64.length();

		/*
		resp->set_body_size(msglen + text.length());
		
		
		// 返回captcha text
		memcpy(&(resp->msg_body.front()), text.c_str(), text.length());

		memcpy(&(resp->msg_body.front()) + text.length(), jpeg_b64.c_str(), jpeg_b64.length());
	
		resp->encode_header(quote::PkgHeader::RES_CAPTCHA, EC_SUCCESS);
		
		sendq_.push(resp);
		*/
}



void trade_server_ssl::error(IMessage* req)
{
	IMessage* resp;// = boost::factory<ssl_response_ptr>()(req->GetSession());
	//resp->set_body_size(0);
	//应该再定义一个类型
	//resp->encode_header(quote::PkgHeader::RES_TRADE, EC_REQ_INVALID);

	sendq_.push(resp);
}

bool trade_server_ssl::GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType)
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
