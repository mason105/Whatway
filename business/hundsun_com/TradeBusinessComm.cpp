

#include "StdAfx.h"
#include "TradeBusinessComm.h"
//#include "LogManager.h"
#include "./connectpool/ConnectManager.h"



#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

//#include "aes.h"

#include "./config/ConfigManager.h"



//#include "CacheData.h"


// ICU
//#include <unicode/putil.h>
//#include <unicode/ucnv.h>

#include "./encrypt/mybotan.h"


#include "./captcha/captchamanager.h"



#include "./output/FileLog.h"

#include "TradeGatewayGtja.h"


TradeBusinessComm::TradeBusinessComm(std::string type)
{


	//m_pConn = NULL;

	aeskey = "29dlo*%AO+3i16BaweTw.lc!)61K{9^5";

	SOH = '\x01';
	LF = '\x1A';

	this->type = type;
}



TradeBusinessComm::~TradeBusinessComm(void)
{
}


void TradeBusinessComm::Process(std::string& request, std::string& response, Trade::TradeLog& logMsg)
{
	/*
	// 开始处理时间
	beginTime =  boost::posix_time::microsec_clock::local_time();

	int nRet = 0;





	logRequest = request;

	

	int rc = 0;

	std::vector<std::string> keyvalues;
	boost::split(keyvalues, request, boost::is_any_of("&")); // 注意需要通过配置文件配置

	// 全部转成小写
	//取的时候也全部改成小写

	for (std::vector<std::string>::iterator it = keyvalues.begin(); it != keyvalues.end(); it++)
	{
		std::string keyvalue = *it;
		TRACE("keyvalue = %s\n", keyvalue.c_str());

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

		if (value!="" &&  value.find("amp;") != std::string::npos)
			boost::algorithm::replace_all(value, "amp;", "&");

		reqmap[key] = value;
	}

	source = reqmap["cssweb_os"];
	if (source == "web" || source == "flash" || source == "ios" || source == "android" || source == "windows")
	{
	}
	else
	{
		if (type == "web" || type == "flash")
		{
			source = type;
		}
		else
		{
			RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, "1001", "cssweb_os参数无效");
			return;
		}
	}

	


	funcid = reqmap["function_id"];
	branchNo = reqmap["op_branch_no"];

//	if (funcid.empty())
//		funcid = reqmap["funcid"];

	

	
	



	long FuncNo = 0;
	long  BranchNo = 0;
	try
	{
		FuncNo = boost::lexical_cast<long>(funcid);

		BranchNo = boost::lexical_cast<long>(branchNo);
	}
	catch(boost::exception& e)
	{
		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, "1001", "缺少参数function_id");
		return;
	}

	OutputDebugString("funcid=");
	OutputDebugString(funcid.c_str());
	OutputDebugString("\n");







	clientIp = "";

	//isSafe = reqmap["isSafe"];

	//flashreqcallback = reqmap["cssweb_type"];
	//TRACE("cssweb_type=%s\n", flashreqcallback.c_str());

	//trdpwd = reqmap["trdpwd"];
	//newpwd = reqmap["newpwd"];


	




	// flash交易解锁
	if (funcid == "000000")
	{
		if (trdpwd != newpwd)
		{
			RetErrRes(Trade::TradeLog::ERROR_LEVEL, response,  "1001", "密码错误，解锁失败！");
			return;
		}
//		else
//			goto UNLOCK_SUCCESS;
	}

	

	m_pConn = g_ConnectManager.GetConnect(sysNo, busiType, branchId);
	if (m_pConn == NULL)
	{
		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, "1000", "连接柜台失败");
		return;
	}
	gt_ip = m_pConn->m_Counter.m_sIP;
	gt_port = boost::lexical_cast<std::string>(m_pConn->m_Counter.m_nPort);

	
		int nFieldCount = 0;
	//nFieldCount = reqmap.count(;

	m_pConn->m_pComm->SetHead(BranchNo, FuncNo);
	m_pConn->m_pComm->SetRange(nFieldCount, 1);

	for (std::map<std::string, std::string>::iterator it = reqmap.begin(); it != reqmap.end(); it++)
	{
		std::string key = it->first;

		if (key == "function_id")
			continue;

		if (key == "cssweb_os")
			continue;

		m_pConn->m_pComm->AddField(key.c_str());

		OutputDebugString("key=");
		OutputDebugString(key.c_str());
		OutputDebugString("\n");
	}

	for (std::map<std::string, std::string>::iterator it = reqmap.begin(); it != reqmap.end(); it++)
	{
		std::string key = it->first;
		std::string value = it->second;

		if (key == "function_id")
			continue;

		if (key == "cssweb_os")
			continue;

		m_pConn->m_pComm->AddValue(value.c_str());

		OutputDebugString("value");
		OutputDebugString("=");
		OutputDebugString(value.c_str());
		OutputDebugString("\n");

	} // end for

		long nSenderId = 1;
	ComputePackID(&nSenderId);

	m_pConn->m_pComm->put_SenderId(nSenderId); // 此处是不是可以改成客户号之类唯一代表客户的值, long类型可以采用唯 一的客户号

	nRet = m_pConn->m_pComm->Send();
	

	if (nRet < 0)
	{
		m_pConn->m_pComm->FreePack();

		long lErrNo = m_pConn->m_pComm->get_ErrorNo();
		std::string sErrMsg = m_pConn->m_pComm->get_ErrorMsg();

		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, boost::lexical_cast<std::string>(lErrNo), sErrMsg);
		return;
	}



	// 接收数据
	

	
	
	
	nRet = m_pConn->m_pComm->Receive();
	if (nRet != 0)
	{
		long lErrNo = m_pConn->m_pComm->get_ErrorNo();
		std::string sErrMsg = m_pConn->m_pComm->get_ErrorMsg();

		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, boost::lexical_cast<std::string>(lErrNo), sErrMsg);

		return;
	}

	long nRecvSenderId = m_pConn->m_pComm->get_SenderId();
	if (nRecvSenderId != nSenderId)
	{
		std::string sErrMsg = "sendid不等于recvid";
		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, boost::lexical_cast<std::string>(0), sErrMsg);

		return;
	}

	nRet = m_pConn->m_pComm->get_ErrorNo();
	if (nRet != 0)
	{
		long lErrNo = m_pConn->m_pComm->get_ErrorNo();
		std::string sErrMsg = m_pConn->m_pComm->get_ErrorMsg();

		RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, boost::lexical_cast<std::string>(lErrNo), sErrMsg);

		return;

	}

	int nCols = m_pConn->m_pComm->get_FieldCount();
	for (int c=0; c<nCols; c++)
	{
		std::string sColName = m_pConn->m_pComm->GetFieldName(c);
		response += sColName;	
					
		if (c != (nCols-1))
		{
			response += SOH;
		}
		else
		{
			response += LF;
		}
	} // end for

	while (m_pConn->m_pComm->get_Eof() == 0)
	{
		int nCols = m_pConn->m_pComm->get_FieldCount();
		for (int c=0; c<nCols; c++)
		{
			std::string fieldName = m_pConn->m_pComm->GetFieldName(c);
			std::string fieldValue = m_pConn->m_pComm->FieldByName(fieldName.c_str());
			
			response += fieldValue;
		
			if (c != (nCols-1))
				response += SOH;

		}
			
		response += LF;


		m_pConn->m_pComm->MoveBy(1);
	} // end while
		


	

	if (response.length() == 0)
	{
		response = "errcode";
		response += SOH;
		response += "errmsg";
		response += LF;
		response += "1";
		response += SOH;
		response += "请求执行成功，没有数据返回！";
		response += LF;
	}
	else
	{
	}		


	// 处理日志
	// 处理日志变量,log函数生成cssweb::csswebmessage


	status = 1;

	this->retcode = "0";

	this->retmsg = "";

	if (response.length() > gConfigManager::instance().m_nResponseLen)
		logResponse = response.substr(0, gConfigManager::instance().m_nResponseLen);
	else
		logResponse = response;

	this->logLevel = Trade::TradeLog::INFO_LEVEL;

	if (source == "web")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nTcpPort);
	}
	else if(source == "ios" || source == "android" || source == "windows")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nSslPort);
	}
	*/
}


void TradeBusinessComm::RetErrRes(Trade::TradeLog::LogLevel logLevel, std::string& response, std::string retcode, std::string retmsg)
{
	//retmsg要过滤=

	if (type == "flash")
	{
	}
	else
	{
		// 需要过滤retmsg
		//boost::algorithm::replace_all(retmsg, "=", ":");

		response = "errcode";
		response += SOH;
		response += "errmsg";
		response += LF;
		response += retcode;
		response += SOH;
		response += retmsg;
		response += LF;
	}





/*
		// convert gbk to utf8
		UErrorCode errcode = U_ZERO_ERROR;
		char dest[1024];
		memset(dest, 0x00, sizeof(dest));
		int ret = ucnv_convert("utf8", "gbk", dest, sizeof(dest), retmsg.c_str(), -1, &errcode);
		row->add_value(dest);
		
		std::string str = "gbk to utf8 长度";
		str += boost::lexical_cast<std::string>(ret);
		str += "\n";

		OutputDebugString(str.c_str());
*/		
	


	// 日志处理


	status = 0;

	this->retcode = retcode;

	this->retmsg = retmsg;

	if (response.length() > gConfigManager::instance().m_nResponseLen)
		logResponse = response.substr(0, gConfigManager::instance().m_nResponseLen);
	else
		logResponse = response;

	this->logLevel = logLevel;

	if (source == "web")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nTcpPort);
	}
	else if(source == "ios" || source == "android" || source == "windows")
	{
		gateway_port = boost::lexical_cast<std::string>(gConfigManager::instance().m_nSslPort);
	}

}

void TradeBusinessComm::ComputePackID(long* lPackID)
{
	/*

	if (g_csPackID.Lock(INFINITE))
	{
	

		if (g_lPackID == (LONG_MAX -1))
			g_lPackID = 1;

		*lPackID = g_lPackID++;

		g_csPackID.Unlock();
	}

	*/
}
