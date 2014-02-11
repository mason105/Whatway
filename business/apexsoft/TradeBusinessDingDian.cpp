

#include "StdAfx.h"
#include "TradeBusinessDingDian.h"
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


#include "./lib/apexsoft/fiddef.h"
#include "./lib/apexsoft/fixapi.h"
#include "./business/apexsoft/DingDian.h"

#include "./cache/CacheData.h"


TradeBusinessDingDian::TradeBusinessDingDian()
{
	m_hHandle = -1;
}

TradeBusinessDingDian::~TradeBusinessDingDian(void)
{
}


bool TradeBusinessDingDian::Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	bool bRet = true;

	std::map<std::string, long> mReturnField;
	int nRet = FALSE;
	std::string fbdm;
	std::string khh;
	long session = 0;

	std::string fid_code = "";
	std::string fid_message = "";
	std::string tmp;


	

	ParseRequest(request);

	// 南京证券禁止修改资料功能
	if ( (sysNo == "njzq_jlp" || sysNo == "njzq_flash") && funcid == "202002")
	{
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

		response += "0";
		response += SOH;
		response += "修改客户资料请到掌厅办理!";
		response += SOH;
		response += "0.0.0.0:5000";
		response += SOH;

		status = 1;
		errCode = "";
		errMsg = "";
		//logLevel = Trade::TradeLog::INFO_LEVEL;

		goto FINISH;
	}

	if (funcid == "000000")
	{
		//1.对解锁密码进行b64解码
		std::string newpwd = reqmap["newpwd"];

		char decoder[20];
		memset(decoder, 0x00, sizeof(decoder));
		int outlen;
		g_MyBotan.Base64Decoder(newpwd, decoder, &outlen);
		std::string unlock = decoder;
		
		//2.对登录密码进行解码，解密
		std::string trdpwd = reqmap["trdpwd"];
		std::string login;
		bool bRet = g_MyBotan.Base16Decoder_AESDecrypt("AES-256/ECB/PKCS7", "29dlo*%AO+3i16BaweTw.lc!)61K{9^5", trdpwd, login);

		//3.比较两个密码
		if (unlock == login)
		{
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

			response += "1";
			response += SOH;
			response += "解锁成功!";
			response += SOH;
			response += "0.0.0.0:5000";
			response += SOH;

			status = 1;
			errCode = "";
			errMsg = "";
			//logLevel = Trade::TradeLog::INFO_LEVEL;

			goto FINISH;
		}
		else
		{
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

			response += "1100";
			response += SOH;
			response += "解锁失败!";
			response += SOH;
			response += "0.0.0.0:5000";
			response += SOH;

			status = 0;
			errCode = "1100";
			errMsg = "解锁失败";
			//logLevel = Trade::TradeLog::INFO_LEVEL;

			goto FINISH;
		}
	}


	
	if (!cssweb_cacheFlag.empty())
	{
		if (g_CacheData.m_mapCacheData.find(cssweb_cacheFlag) != g_CacheData.m_mapCacheData.end())
		{
			std::string value = g_CacheData.m_mapCacheData[cssweb_cacheFlag];
		
			if (!value.empty())
			{
				response = value;

				status = 1;
				errCode = "";
				errMsg = "";
				//logLevel = Trade::TradeLog::INFO_LEVEL;

				gFileLog::instance().Log("cache use " + cssweb_cacheFlag);
				goto FINISH;
			}
		}
	}
	

	// 判断功能号是否已定义
	if (g_DingDian.m_mReturnField.find(funcid) == g_DingDian.m_mReturnField.end())
	{
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

			response += "1100";
			response += SOH;
			response += "功能号没有配置!";
			response += SOH;
			response += "0.0.0.0:5000";
			response += SOH;

			status = 0;
			errCode = "2001";
			errMsg = "功能号没有配置";
			//logLevel = Trade::TradeLog::ERROR_LEVEL;

			goto FINISH;
	}

	
	
	
	session = Fix_AllocateSession(m_hHandle);
	
	// set gydm
	//gFileLog::instance().Log("set gydm:" + m_pConn->m_Counter->m_sGydm);
	Fix_SetGYDM(session, m_Counter->m_sGydm.c_str());
	

	// set fbdm & dest fbdm
	
	
	khh = reqmap["FID_KHH"];
	if (!khh.empty())
	{
		 fbdm = khh.substr(0,4);
	}
	else
	{
		std::string zjzh = reqmap["FID_ZJZH"];
		fbdm = zjzh.substr(0,4);
	}
	//gFileLog::instance().Log("set fbdm:"+fbdm);
	Fix_SetFBDM(session, fbdm.c_str());
	Fix_SetDestFBDM(session, fbdm.c_str());



	// set wtfs
	
	if (sysVer == "iphone" || sysVer == "aphone")
	{
		//gFileLog::instance().Log("set wtfs:"+m_pConn->m_Counter->m_sWtfs_mobile);
		Fix_SetWTFS(session, m_Counter->m_sWtfs_mobile.c_str());
	}
	else
	{
		//RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, "1009", "委托方式参数错误");
		//goto FINISH;
		Fix_SetWTFS(session, m_Counter->m_sWtfs_web.c_str());

	}



	// set node
	
	if (sysVer == "iphone" || sysVer == "aphone")
	{
		std::string node;
		node =reqmap["cssweb_note"];

		//前端格式18621112013
		if (node.find("ZRSJ") == std::string::npos)
			node = "ZRSJ:" + node;

	
		Fix_SetNode(session, node.c_str());
	}


	//gFileLog::instance().Log("Fix_CreateHead" + funcid);
	Fix_CreateHead(session, boost::lexical_cast<long>(funcid));

	
	for (std::map<std::string, std::string>::iterator it = reqmap.begin(); it != reqmap.end(); it++)
	{
		std::string key = it->first;
		std::string value = it->second;


		if (FilterRequestField(key))
		{
			continue;
		}
		else if(key == "client_id")
		{
			continue;
		}
		else if(key == "branch_no")
		{
			continue;
		}
		else if (key == "FBDM")
		{
			Fix_SetFBDM(session, value.c_str());
			Fix_SetDestFBDM(session, value.c_str());
		}
		else if (key == "FID_JYMM")
		{

			
			// 登录
			if (funcid == "190101")
			{
				if (sysVer == "web" || sysVer == "flash")
				{
					//加了FID_JMLX=2，所有不需要加密传给柜台
					std::string pwd;
	
					bool bRet = g_MyBotan.Base16Decoder_AESDecrypt("AES-256/ECB/PKCS7", "29dlo*%AO+3i16BaweTw.lc!)61K{9^5", value, pwd);

					long l = g_DingDian.m_mRequestField[key];
					Fix_SetItem(session, l, pwd.c_str());
				}
				else
				{
					// iphone, aphone
					//没加FID_JMLX=2，所以需要加密传给柜台

					char szPwd[256];
					memset(szPwd, 0x00, sizeof(szPwd));

					EncryptPwd(value.c_str(), szPwd);
					if (szPwd != NULL)
					{
						long l = g_DingDian.m_mRequestField[key];
						Fix_SetItem(session, l, szPwd);
					}
				}
			}
			else
			{
				char szPwd[256];
				memset(szPwd, 0x00, sizeof(szPwd));

				EncryptPwd(value.c_str(), szPwd);
				if (szPwd != NULL)
				{
					long l = g_DingDian.m_mRequestField[key];
					Fix_SetItem(session, l, szPwd);
				}
			}
		}
		else if(key == "FID_MM" || key == "FID_NEWMM" || key == "FID_ZJMM" || key == "FID_WBZHMM")
		{
			if (sysVer == "flash")
			{
				
				if (cssweb_pwdType == "1")
				{
					// 安全模式
					std::string pwd;
					bool bRet = g_MyBotan.Base16Decoder_AESDecrypt("AES-256/ECB/PKCS7", "29dlo*%AO+3i16BaweTw.lc!)61K{9^5", value, pwd);

					char szPwd[256];
					memset(szPwd, 0x00, sizeof(szPwd));

					EncryptPwd(pwd.c_str(), szPwd);
					if (szPwd != NULL)
					{
						long l = g_DingDian.m_mRequestField[key];
						Fix_SetItem(session, l, szPwd);
					}
				}
				else
				{
					// 普通模式
					char decoder[20];
					memset(decoder, 0x00, sizeof(decoder));
					int outlen;
					bool bRet = g_MyBotan.Base64Decoder(value, decoder, &outlen);

					std::string pwd = decoder;

					char szPwd[256];
					memset(szPwd, 0x00, sizeof(szPwd));

					EncryptPwd(pwd.c_str(), szPwd);
					if (szPwd != NULL)
					{
						long l = g_DingDian.m_mRequestField[key];
						Fix_SetItem(session, l, szPwd);
					}

					if (key == "FID_NEWMM")
					{
						// 新密码
						bool bRet = g_MyBotan.AESEncrypt_Base16Encoder("AES-256/ECB/PKCS7", "29dlo*%AO+3i16BaweTw.lc!)61K{9^5", pwd, flash_normal_modifypwd_newpwd);
						gFileLog::instance().Log("新密码"+pwd+"，加密" + flash_normal_modifypwd_newpwd);
					}
				}
			}
			else if (sysVer == "web")
			{
					// 安全模式
					std::string pwd;
					bool bRet = g_MyBotan.Base16Decoder_AESDecrypt("AES-256/ECB/PKCS7", "29dlo*%AO+3i16BaweTw.lc!)61K{9^5", value, pwd);

					char szPwd[256];
					memset(szPwd, 0x00, sizeof(szPwd));

					EncryptPwd(pwd.c_str(), szPwd);
					if (szPwd != NULL)
					{
						long l = g_DingDian.m_mRequestField[key];
						Fix_SetItem(session, l, szPwd);
					}
			}
			else
			{
				char szPwd[256];
				memset(szPwd, 0x00, sizeof(szPwd));

				EncryptPwd(value.c_str(), szPwd);
				if (szPwd != NULL)
				{
					long l = g_DingDian.m_mRequestField[key];
					Fix_SetItem(session, l, szPwd);
				}
			}
		}
		else
		{
			//if (g_DingDian.m_mRequestField.find(key) == g_DingDian.m_mRequestField.end())
			//{
			//	RetErrRes(Trade::TradeLog::ERROR_LEVEL, response, "2000", "字段值在tradeparam.map中无法找到");
			//	goto FINISH;
			//}
			//else
			//{
				long l = g_DingDian.m_mRequestField[key];
				//gFileLog::instance().Log("Fix_SetItem:" + key);
				//TRACE("%s=%s\n", key.c_str(),  value.c_str());
				Fix_SetItem(session, l, value.c_str());
			//}
		}
	} // end for

	// 默认为30秒
	Fix_SetTimeOut(session, m_Counter->m_nRecvTimeout);

	nRet = Fix_Run(session);
	if (!nRet)
	{
		long lErrCode = Fix_GetCode(session);
		char szErrMsg[1024];
		Fix_GetErrMsg(session, szErrMsg, sizeof(szErrMsg));

		GenResponse(lErrCode, szErrMsg, response, status, errCode, errMsg);

		goto FINISH;
	}	



	char szValue[512];

	memset(szValue, '\0', sizeof(szValue));
	Fix_GetItem(session, FID_CODE, szValue, sizeof(szValue), 0);
	fid_code = szValue;
	

	memset(szValue, '\0', sizeof(szValue));
	Fix_GetItem(session, FID_MESSAGE, szValue, sizeof(szValue), 0);
	fid_message = szValue;
	
	tmp = "功能号：" + funcid + "fid_code: " + fid_code;
	gFileLog::instance().Log(tmp);

	int fidcode = -1;
	if (!fid_code.empty())
	{
		fidcode = boost::lexical_cast<int>(fid_code);
	}

	if (fid_code.empty() || fidcode >= 0)
	{
		
		long nRows = Fix_GetCount(session);
		//gFileLog::instance().Log("Fix_GetCount");

		// 处理成功，无数据返回
		if (nRows == 0)
		{
			RetNoRecordRes(response);
			goto FINISH;
		}

		//gFileLog::instance().Log("开始处理结果集");
		mReturnField = g_DingDian.m_mReturnField[funcid];
		int nCols = mReturnField.size();

		//flash交易，普通模式，修改密码功能
		if (sysVer=="flash" && cssweb_pwdType == "0" && funcid=="202010")
		{
			nCols += 1;
		}

		response = boost::lexical_cast<std::string>(nRows);
		response += SOH;
		response += boost::lexical_cast<std::string>(nCols);
		response += SOH;

		


		// 列头
		for (std::map<std::string, long>::iterator it = mReturnField.begin(); it != mReturnField.end(); it++)
		{
			std::string sColName = it->first;

			response += sColName;
			response += SOH;
		}

		//flash交易，普通模式，修改密码功能
		if (sysVer=="flash" && cssweb_pwdType == "0" && funcid=="202010")
		{
			response += "cssweb_pwd";
			response += SOH;
		}


		for (int row=0; row<nRows; row++)
		{
			char szValue[512];
			for (std::map<std::string, long>::iterator it = mReturnField.begin(); it != mReturnField.end(); it++)
			{
				std::string sKey = it->first;
				long lKey = it->second;

			
				memset(szValue, '\0', sizeof(szValue));
				Fix_GetItem(session, lKey, szValue, sizeof(szValue), row);

				response += szValue;
				response += SOH;
			} // end for col
		} // end for row

		//flash交易，普通模式，修改密码功能
		if (sysVer=="flash" && cssweb_pwdType == "0" && funcid=="202010")
		{
			response += flash_normal_modifypwd_newpwd;
			response += SOH;
		}

		status = 1;
		errCode = "";
		errMsg = "";
		//logLevel = Trade::TradeLog::INFO_LEVEL;

		// 结果集缓存
		if (!cssweb_cacheFlag.empty())
		{
			gFileLog::instance().Log("加入缓存" + cssweb_cacheFlag);
			g_CacheData.m_mapCacheData[cssweb_cacheFlag] = response;
			
		}
	}
	else
	{
		GenResponse(fidcode, fid_message, response, status, errCode, errMsg);
		goto FINISH;
	}






FINISH:
	
	if (session != 0)
	{
		Fix_ReleaseSession(session);
	}

	return bRet;

}


char* TradeBusinessDingDian::EncryptPwd(const char* plainText, char* szPwd)
{

	if (plainText == NULL)
		return NULL;

	strcpy(szPwd, plainText);
	Fix_Encode(szPwd);
	return szPwd;
}


bool TradeBusinessDingDian::CreateConnect()
{
	

	int nRet = 0;

	

		std::string gtAddr = m_Counter->m_sIP + "@" + boost::lexical_cast<std::string>(m_Counter->m_nPort) + "/tcp";
		m_hHandle = Fix_Connect(gtAddr.c_str(), m_Counter->m_sUserName.c_str(), m_Counter->m_sPassword.c_str(), m_Counter->m_nConnectTimeout); 

		
		if (m_hHandle == 0)
		{
			return false;
		}
		else
		{
			return true;
		}

}

void TradeBusinessDingDian::CloseConnect()
{
	if (m_hHandle != -1)
	{
		Fix_Close(m_hHandle);
	}
}
