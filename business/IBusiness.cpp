#include "IBusiness.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "./config/ConfigManager.h"
#include "./output/FileLog.h"
#include "./connectpool/ConnectManager.h"
#include "./encrypt/MyBotan.h"


IBusiness::IBusiness(void)
{
	m_bConnected = false;

	SOH = "\x01";
}


IBusiness::~IBusiness(void)
{
}

void IBusiness::SetCounterServer(Counter * counter)
{
	this->m_Counter = counter;
}

int IBusiness::ConvertIntToBusiType(int val)
{
	switch (val)
	{
	case 1:
		return BUSI_TYPE_STOCK;
	case 2:
		return BUSI_TYPE_CREDIT;
	case 3:
		return BUSI_TYPE_REGISTER;
	case 4:
		return BUSI_TYPE_ACCOUNT;
	case 5:
		return BUSI_TYPE_AUTH;
	case 6:
		return BUSI_TYPE_OPTION;
	case 7:
		return BUSI_TYPE_VERIFY;
	case 0:
	default:
		return BUSI_TYPE_ALL;

	}
}

void IBusiness::ParseRequest(std::string& request)
{
	std::vector<std::string> keyvalues;
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

//		if (value!="" &&  value.find("amp;") != std::string::npos)
//			boost::algorithm::replace_all(value, "amp;", "&");

		reqmap[key] = value;
	}

	sysNo = reqmap["cssweb_sysNo"];

	bt = reqmap["cssweb_busiType"];
	if (!bt.empty())
		busiType = ConvertIntToBusiType(boost::lexical_cast<int>(bt));

	sysVer = reqmap["cssweb_sysVer"];
	//CheckParam("sysver", sysver);
	funcid = reqmap["cssweb_funcid"];
	route = reqmap["cssweb_route"];
	hardinfo = reqmap["cssweb_hardinfo"];


	captcha = reqmap["cssweb_captcha"];
	
	note = reqmap["cssweb_note"];


	branchId = reqmap["branch_no"]; // 营业部id
	account = reqmap["client_id"]; // 客户号

	cssweb_cacheFlag = reqmap["cssweb_cacheFlag"];

	cssweb_pwdType = reqmap["cssweb_pwdType"];
	cssweb_flashCallback = reqmap["cssweb_flashCallback"];

	cssweb_connid = reqmap["cssweb_connid"];
}

bool IBusiness::FilterRequestField(std::string& key)
{
	if (key == "cssweb_sysNo")
	{
		return true;
	}
	else if (key == "cssweb_busiType")
	{
		return true;
	}
	else if (key == "cssweb_sysVer")
	{
		return true;
	}
	else if (key == "cssweb_funcid")
	{
		return true;
	}
	else if (key == "cssweb_route")
	{
		return true;
	}
	else if (key == "cssweb_hardinfo")
	{
		return true;
	}
	else if (key == "cssweb_captcha")
	{
		return true;
	}
	else if (key == "cssweb_note")
	{
		return true;
	}
	else if (key == "cssweb_cacheFlag")
	{
		return true;
	}
	else if (key == "cssweb_ram")
	{
		return true;
	}
	else if (key == "cssweb_flashCallback")
	{
		return true;
	}
	else if (key == "cssweb_pwdType")
	{
		return true;
	}
	else if (key == "cssweb_connid")
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
void IBusiness::BeginLog(std::string& request)
{
	beginTime =  boost::posix_time::microsec_clock::local_time();
	sBeginTime = boost::gregorian::to_iso_extended_string(beginTime.date()) + " " + boost::posix_time::to_simple_string(beginTime.time_of_day());

	logRequest = request;
}
*/

void IBusiness::RetErrRes(Trade::TradeLog::LogLevel level, std::string& response, std::string code, std::string msg)
{
	//retmsg要过滤=

		// 需要过滤retmsg
		//boost::algorithm::replace_all(retmsg, "=", ":");

	response = "1";
	response += SOH;
	response += "3";
	//response += "4";
	response += SOH;

	response += "cssweb_code";
	response += SOH;
	response += "cssweb_msg";
	response += SOH;
//	response += "cssweb_flashCallback";
//	response += SOH;
	response += "cssweb_gwInfo";
	response += SOH;

	response += code;
	response += SOH;
	response += msg;
	response += SOH;
//	response += cssweb_flashCallback;
//	response += SOH;
	response += "0.0.0.0:5000";
	response += SOH;

	status = 0;
	logLevel = level;
	retcode = code;
	retmsg = msg;
}



void IBusiness::RetNoRecordRes(std::string& response)
{
	response = "1";
	response += SOH;
	response += "3";
	response += SOH;

	response += "cssweb_code";
	response += SOH;
	response += "cssweb_msg";
	response += SOH;
//	response += "cssweb_flashCallback";
//	response += SOH;
	response += "cssweb_gwInfo";
	response += SOH;

	response += "1";
	response += SOH;
	response += "请求成功，没有结果集返回。";
	response += SOH;
//	response += cssweb_flashCallback;
//	response += SOH;
	response += "0.0.0.0:5000";
	response += SOH;

	logLevel = Trade::TradeLog::INFO_LEVEL;	
	status = 1;
	retcode = "";
	retmsg = "";
}

/*
void IBusiness::EndLog(std::string& response, Trade::TradeLog& logMsg)
{
	// 得到所有日志相关变量值
	boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = endTime - beginTime;
	runtime = td.total_milliseconds();
	if (runtime == 0)
	{
		std::string sLog = "功能 " + funcid + " 运行时间为0异常";
		gFileLog::instance().Log(sLog);
	}



	if (response.length() > gConfigManager::instance().m_nResponseLen)
		logResponse = response.substr(0, gConfigManager::instance().m_nResponseLen);
	else
		logResponse = response;

	

	if (sysVer == "web")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nTcpPort);
	}
	else if (sysVer == "flash")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nHttpPort);
	}
	else if(sysVer == "windows" || sysVer == "iphone" || sysVer == "ipad" || sysVer == "aphone" || sysVer == "apad")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nSslPort);
	}

	gateway_ip = gConfigManager::instance().m_sIp;
	 


	// 填充结构体
	logMsg.set_sysno(sysNo);
	logMsg.set_sysver(sysVer);	
	logMsg.set_busitype(bt);
	logMsg.set_account(account);
	logMsg.set_level(logLevel);
	
	logMsg.set_funcid(funcid);
	logMsg.set_countertype(sCounterType);

	logMsg.set_begintime(sBeginTime); // begintime
	logMsg.set_runtime(runtime);


	logMsg.set_ip(note);
	logMsg.set_request(logEncodeRequest); // flash
	logMsg.set_request(logRequest);
	logMsg.set_status(status);
	logMsg.set_retcode(retcode);
	logMsg.set_retmsg(retmsg);
	logMsg.set_response(logResponse);
	logMsg.set_enc_response(logEncodeResponse); // flash
	
	logMsg.set_gatewayip(gateway_ip);
	logMsg.set_gatewayport(gateway_port);
	logMsg.set_gtip(gt_ip);
	logMsg.set_gtport(gt_port);

}
*/
//		std::string algo = "AES-256/ECB/PKCS7";
	//	std::string pubKey = "23dpasd23d-23l;df9302hzv/3lvjh*5";

bool IBusiness::DecryptPassword(std::string algo, std::string key, std::string cipher, std::string plain)
{
	// 因为base64有可能产生=，会打乱原先的格式
	boost::algorithm::replace_all(cipher, "$", "=");

	if (cssweb_pwdType == "0")
	{
		// 普通模式
		
		int outlen;
		char decoder[50];
		memset(decoder, 0x00, sizeof(decoder));
	
		bool bRet = g_MyBotan.Base64Decoder(cipher, decoder, &outlen);
		plain = decoder;

		return bRet;
	}
	else if (cssweb_pwdType == "1")
	{
		// 安全模式
	}
	else
		return false;

	return false;
}
void IBusiness::GenResponse(int nErrCode, std::string sErrMsg, std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	status = 0;

	errCode = boost::lexical_cast<std::string>(nErrCode);
	errMsg = sErrMsg;

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

	response += errCode;
	response += SOH;
	response += errMsg;
	response += SOH;
	response += "0.0.0.0:5000";
	response += SOH;
}


bool IBusiness::IsConnected()
{
	
	return m_bConnected;
}

/*
std::string TradeBusinessT2::GetTradePWD(std::string isSafe, std::string sEncPwd)
{

	// 因为base64有可能产生=，会打乱原先的格式
	boost::algorithm::replace_all(sEncPwd, "$", "=");

	std::string sPwd = "";

	if (isSafe == "0")
	{
		// 标准登录采用weblogic密钥解密
		std::string algo = "AES-256/ECB/PKCS7";
		std::string pubKey = "23dpasd23d-23l;df9302hzv/3lvjh*5";
		std::string cipher = sEncPwd;
		
		bool bRet = g_MyBotan.Base64Decoder_AESDecrypt(algo, pubKey, cipher, sPwd);
	}
	else if (isSafe == "1")
	{
		// 加强登录采用控件密钥解密
		sPwd = GetOtherPWD("1", sEncPwd);
	}
	else
	{
		// windows phone
		return sEncPwd;
	}

	return sPwd;
}

std::string TradeBusinessT2::GetOtherPWD(std::string isSafe, std::string sEncPwd)
{
	// 因为base64有可能产生=，会打乱原先的格式
	boost::algorithm::replace_all(sEncPwd, "$", "=");

	std::string sPwd = "";

	if (isSafe == "0")
	{
		char decoder[50];
		memset(decoder, 0x00, sizeof(decoder));
		int outlen;

		g_MyBotan.Base64Decoder(sEncPwd, decoder, &outlen);
		sPwd = decoder;
	}
	else if (isSafe == "1")
	{
			// 加强登录采用控件密钥解密
		

		int outlen;
		char out[256];
		memset(out, 0, sizeof(out));
		//int outlen = sEncPwd.length()/2;
		//outlen = HexDataToBinData((unsigned char*)sEncPwd.c_str(), sEncPwd.length(), (unsigned char*)out, sizeof(out));


		std::string algo = "AES-256/ECB/PKCS7";
		std::string pubKey = "29dlo*%AO+3i16BaweTw.lc!)61K{9^5";
		bool bRet = g_MyBotan.AESDecrypt(algo, pubKey, (const unsigned char*)out, outlen, sPwd);


	}
	else
	{
		// windows phone
		return sEncPwd;
	}

	return sPwd;
}

std::string TradeBusiness::GetTradePWD(std::string isSafe, std::string sEncPwd)
{

	// 因为base64有可能产生=，会打乱原先的格式
	boost::algorithm::replace_all(sEncPwd, "$", "=");

	std::string sPwd = "";

	if (isSafe == "0")
	{
		// 标准登录采用weblogic密钥解密
		std::string algo = "AES-256/ECB/PKCS7";
		std::string pubKey = "23dpasd23d-23l;df9302hzv/3lvjh*5";
		std::string cipher = sEncPwd;
		
		bool bRet = g_MyBotan.Base64Decoder_AESDecrypt(algo, pubKey, cipher, sPwd);
	}
	else if (isSafe == "1")
	{
		// 加强登录采用控件密钥解密
		sPwd = GetOtherPWD("1", sEncPwd);
	}
	else
	{
		// windows phone
		return sEncPwd;
	}

	return sPwd;
}

std::string TradeBusiness::GetOtherPWD(std::string isSafe, std::string sEncPwd)
{
	// 因为base64有可能产生=，会打乱原先的格式
	boost::algorithm::replace_all(sEncPwd, "$", "=");

	std::string sPwd = "";

	if (isSafe == "0")
	{
		char decoder[50];
		memset(decoder, 0x00, sizeof(decoder));
		int outlen;

		g_MyBotan.Base64Decoder(sEncPwd, decoder, &outlen);
		sPwd = decoder;
	}
	else if (isSafe == "1")
	{
			// 加强登录采用控件密钥解密
		

		int outlen;
		char out[256];
		memset(out, 0, sizeof(out));
		//int outlen = sEncPwd.length()/2;
		//outlen = HexDataToBinData((unsigned char*)sEncPwd.c_str(), sEncPwd.length(), (unsigned char*)out, sizeof(out));


		std::string algo = "AES-256/ECB/PKCS7";
		std::string pubKey = "29dlo*%AO+3i16BaweTw.lc!)61K{9^5";
		bool bRet = g_MyBotan.AESDecrypt(algo, pubKey, (const unsigned char*)out, outlen, sPwd);


	}
	else
	{
		// windows phone
		return sEncPwd;
	}

	return sPwd;
}
*/