#include "stdafx.h"


#include <string>
#include <boost/format.hpp>

#include "Connect.h"
#include "./config/ConfigManager.h"
#include "common.h"
#include "./output/FileLog.h"

#include "./lib/szkingdom/KDEncodeCli.h"
#include "./lib/szkingdom/kcbpcli.h"

// 顶点

#include "./lib/apexsoft/fixapi.h"
#include "./lib/apexsoft/fiddef.h"

#include "./business/apexsoft/DingDian.h"


Connect::Connect(int ConnectNo, Counter counter)
{
	init();

	m_nID = ConnectNo;
	m_Counter = counter;

	boost::format fmt("序号%1% 柜台地址%2%:%3%");
	fmt % m_nID % m_Counter.m_sIP % m_Counter.m_nPort;
	m_sServerInfo = fmt.str();

}

void Connect::init()
{
	// 金证windows
	handle = NULL;
	
	//  恒生t2
	lpConnection = NULL;
	lpConfig = NULL;

	// 恒生COM
	m_pComm = NULL;

	// 顶点
	m_hHandle = 0;

	

	
}

std::string Connect::GetConnectInfo()
{
	return m_sServerInfo;
}

Connect::~Connect(void)
{
	CloseConnect();
}



bool Connect::CreateConnectT2()
{
	int nRetry = gConfigManager::instance().m_nConnectRetry;


	int nRet = 0;

	lpConfig = NewConfig();
	lpConfig->AddRef();

	std::string s = m_Counter.m_sIP;
	s += ":";
	s += boost::lexical_cast<std::string>(m_Counter.m_nPort);
	lpConfig->SetString("t2sdk", "servers", s.c_str());

	std::string license_file;
	license_file = gConfigManager::instance().m_sPath + "\\license.dat";
	lpConfig->SetString("t2sdk", "license_file", license_file.c_str());
	lpConfig->SetString("t2sdk", "lang", "1033");
	lpConfig->SetString("t2sdk", "send_queue_size", "100");
	lpConfig->SetString("safe", "safe_level", "none");

	for (int i=0; i<nRetry; i++)
	{
		lpConnection = NewConnection(lpConfig);
		lpConnection->AddRef();

		nRet = lpConnection->Create(NULL);

		
		nRet = lpConnection->Connect(m_Counter.m_nConnectTimeout*1000);

		if (nRet != 0)
		{
			// 连接连接失败
			std::string sErrMsg = lpConnection->GetErrorMsg(nRet);
			std::string msg = "建立连接失败, " + m_sServerInfo + sErrMsg;
			gFileLog::instance().Log(msg);

			Sleep(500);

			continue;
		}
		else
		{
			// 设置连接创建时间
			prev = time(NULL);

			std::string msg = "建立连接成功, " + m_sServerInfo;
			
			gFileLog::instance().Log(msg);

			return true;
		}
	} // end for

	return false;
}

bool Connect::CreateConnectKCBP()
{
	int nRetry = gConfigManager::instance().m_nConnectRetry;


	int nRet = 0;

	for (int i=0; i<nRetry; i++)
	{
		tagKCBPConnectOption stKCBPConnection;
		memset(&stKCBPConnection, 0x00, sizeof(stKCBPConnection));
		strcpy(stKCBPConnection.szServerName, m_Counter.m_sServerName.c_str());
		stKCBPConnection.nProtocal = 0;
		strcpy(stKCBPConnection.szAddress, m_Counter.m_sIP.c_str());
		stKCBPConnection.nPort = m_Counter.m_nPort;
		strcpy(stKCBPConnection.szSendQName, m_Counter.m_sReq.c_str());
		strcpy(stKCBPConnection.szReceiveQName, m_Counter.m_sRes.c_str());

		nRet = KCBPCLI_Init( &handle );
		nRet = KCBPCLI_SetConnectOption( handle, stKCBPConnection );		

		//设置超时
		nRet = KCBPCLI_SetCliTimeOut( handle, m_Counter.m_nConnectTimeout);

		// 设置是否输出调试信息
		//nRet = KCBPCLI_SetOptions( handle, KCBP_OPTION_TRACE, &gConfigManager::instance().m_nIsTradeServerDebug, sizeof(int));

		nRet = KCBPCLI_SQLConnect( handle, stKCBPConnection.szServerName, (char*)m_Counter.m_sUserName.c_str(), (char*)m_Counter.m_sPassword.c_str());

		if (nRet != 0)
		{
			// 连接连接失败
			std::string msg = "建立连接失败, " + m_sServerInfo + ",错误码：" + boost::lexical_cast<std::string>(nRet);
			

			gFileLog::instance().Log(msg);

			Sleep(500);

			continue;
		}
		else
		{
			// 设置连接创建时间
			prev = time(NULL);

			std::string msg = "建立连接成功, " + m_sServerInfo;
			
			gFileLog::instance().Log(msg);

			return true;
		}
	} // end for

	return false;
}

bool Connect::CreateConnectComm()
{
	::CoInitialize(NULL);

	int nRetry = gConfigManager::instance().m_nConnectRetry;


	int nRet = 0;

	for (int i=0; i<nRetry; i++)
	{
		m_pComm = new CComm();
		m_pComm->CreateDispatch("HsCommX.Comm");
		m_pComm->Create();
		
		//handle->put_SendTimeOut(g_ConfigManager.m_nSendTimeout);
		//handle->put_ReceiveTimeOut(g_ConfigManager.m_nRecvTimeout);

		nRet = m_pComm->ConnectX(1, m_Counter.m_sIP.c_str(), m_Counter.m_nPort, 0, "", 0);
		if (nRet != 0)
		{
			// 连接连接失败
			std::string msg = "建立连接失败, " + m_sServerInfo + "\n";
			OutputDebugString(msg.c_str());

			//g_LogManager.SendNormalMsg(Cssweb::CsswebMessage::ERROR_LEVEL, Cssweb::CsswebMessage::GUI_FILE_OUTPUT, "", msg);

			Sleep(500);

			continue;
		}
		else
		{
			// 设置连接创建时间
			prev = time(NULL);

			std::string msg = "建立连接成功, " + m_sServerInfo + "\n";
			//g_LogManager.SendNormalMsg(Cssweb::CsswebMessage::DEBUG_LEVEL, Cssweb::CsswebMessage::GUI_FILE_OUTPUT, "", msg);
			OutputDebugString(msg.c_str());

			return true;
		}
	} // end for

	return false;
}

bool Connect::CreateConnectDingDian()
{
	int nRetry = gConfigManager::instance().m_nConnectRetry;


	int nRet = 0;

	
	for (int i=0; i<nRetry; i++)
	{
		std::string gtAddr = m_Counter.m_sIP + "@" + boost::lexical_cast<std::string>(m_Counter.m_nPort) + "/tcp";
		m_hHandle = Fix_Connect(gtAddr.c_str(), m_Counter.m_sUserName.c_str(), m_Counter.m_sPassword.c_str(), m_Counter.m_nConnectTimeout); 

		
		if (m_hHandle == 0)
		{
			// 连接连接失败
			std::string msg = "建立连接失败, " + m_sServerInfo;
			gFileLog::instance().Log(msg);

			Sleep(500);

			continue;
		}
		else
		{
			// 设置连接创建时间
			prev = time(NULL);

			std::string msg = "建立连接成功, " + m_sServerInfo;
			
			gFileLog::instance().Log(msg);

			return true;
		}
	} // end for
	

	return false;
}


bool Connect::CreateConnect()
{
	bool bRet = false;

	switch(m_Counter.m_eCounterType)
	{
	case CT_HS_T2:
		bRet = CreateConnectT2();
		break;
	case CT_HS_COM:
		bRet = CreateConnectComm();
		break;
	case CT_JZ_WIN:
		bRet = CreateConnectKCBP();
		break;
	case CT_JZ_LINUX:
		break;
	case CT_DINGDIAN:
		bRet = CreateConnectDingDian();
		break;
	
	default:
		break;
	}

	return bRet;
}

bool Connect::IsTimeout()
{
	// 当前时间
	time_t now = time(NULL);

	// 柜台api没有提供当前连接是否有效的方法
	// 无法拿到柜台的socket，无法采用tcp保活机制来实现维持连接
	// 由于tcp连接在一定时间内没有数据流量会中断，所以需要判断是否超时
	// prev是连接上次产生流量的时间
	if ((now - prev) >= m_Counter.m_nIdleTimeout)
	{
		std::string msg = "连接超时, " + m_sServerInfo;
		gFileLog::instance().Log(msg);
		
		return true;
	}

	//std::string msg = "连接没有超时" + m_sServerInfo;
	return false;
}


bool Connect::ReConnect()
{
	CloseConnect();

	bool bRet = CreateConnect();

	if (bRet)
	{
		std::string msg = "重连成功, " + m_sServerInfo;
		gFileLog::instance().Log(msg);
	}
	else
	{
		std::string msg = "重连失败, " + m_sServerInfo;
		gFileLog::instance().Log(msg);
	}

	return bRet;
}



void Connect::CloseConnect()
{
		
	switch(m_Counter.m_eCounterType)
	{
	case CT_HS_T2:
		// 恒生t2sdk
		if (lpConnection != NULL)
		{
			std::string msg = "关闭恒生t2连接，" + m_sServerInfo;
			gFileLog::instance().Log(msg);

			lpConnection->Release();
			//delete lpConnection;
			lpConnection = NULL;

		}

		if (lpConfig != NULL)
		{
			lpConfig->Release();
			//delete lpConfig;
			lpConfig = NULL;
		}
		break;
	case CT_HS_COM:
		// 恒生COM
		if (m_pComm != NULL)
		{
			std::string msg = "关闭恒生COM连接，" + m_sServerInfo;
			gFileLog::instance().Log(msg);

			m_pComm->DisConnect();
			m_pComm->ReleaseDispatch();
			m_pComm->Free();
			delete m_pComm;
			m_pComm = NULL;

		}
		break;
	case CT_JZ_WIN:
		// 金证windows版本
		if (handle != NULL)
		{
			std::string msg = "关闭金证win连接，" + m_sServerInfo;
			gFileLog::instance().Log(msg);

			KCBPCLI_SQLDisconnect(handle);
			KCBPCLI_Exit(handle);
			handle = NULL;
		}
		break;
	case CT_JZ_LINUX:
		break;
	case CT_DINGDIAN:
		// 顶点
		
		if (m_hHandle != NULL)
		{
			std::string msg = "关闭顶点连接，" + m_sServerInfo;
			gFileLog::instance().Log(msg);

			Fix_Close(m_hHandle);
			m_hHandle = NULL;
		}
		
		break;
	
	default:
		break;
	}
}
