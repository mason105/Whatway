#include "stdafx.h"
//#include <windows.h>



#include <list>
#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include "./lib/xml/pugixml.hpp"

#include "ConfigManager.h"
#include "./connectpool/counter.h"
#include "./connectpool/ConnectManager.h"
#include "./connectpool/ConnectPool.h"



using namespace boost::property_tree;
using namespace boost::filesystem;




ConfigManager::ConfigManager(void)
{
}

ConfigManager::~ConfigManager(void)
{
}

bool ConfigManager::LoadConfig(std::string sPath)
{
	if (sPath.empty())
	{
		char szPath[255];
		memset(szPath, 0x00, sizeof(szPath));
		::GetModuleFileName(NULL, szPath, sizeof(szPath));
		m_sExeFile = szPath;

		boost::filesystem::path p(szPath);

	
		m_sPath = p.parent_path().string();
	}
	else
	{
		m_sPath = sPath;
	}

	std::string sConfigFile = m_sPath + "\\tradegateway.xml";
	



/*

	boost::format fmt("%1%:%2%");
	fmt % m_sIPAddr % m_nTcpPort;
	m_sWebTradeAddr = fmt.str();


*/

	



	pugi::xml_document doc;
	if (!doc.load_file(sConfigFile.c_str()))
		return false;

	pugi::xpath_node node;
	std::string node_path;
	int node_size;
	pugi::xpath_node_set nodes;
	int enable;
	std::string node_value;

	// 通信层
	node = doc.select_single_node("/config/communication/ip");
	m_sIp = node.node().child_value();

	node = doc.select_single_node("/config/communication/ssl/port");
	m_nSslPort = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/ssl/workerthreadpool");
	m_nSslWorkerThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/ssl/sendthreadpool");
	m_nSslSendThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/ssl/auth");
	m_nAuth = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/communication/http/port");
	m_nHttpPort = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/http/workerthreadpool");
	m_nHttpWorkerThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/http/sendthreadpool");
	m_nHttpSendThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/http/flashurlcontext");
	m_sFlashUrlContext = node.node().child_value();

	node = doc.select_single_node("/config/communication/tcp/port");
	m_nTcpPort = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/tcp/workerthreadpool");
	m_nTcpWorkerThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/communication/tcp/sendthreadpool");
	m_nTcpSendThreadPool = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/communication/zlib");
	zlib = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/communication/RunMode");
	nRunMode = boost::lexical_cast<int>(node.node().child_value());


	// 柜台共用
	node = doc.select_single_node("/config/Counter_Common/connectpool/min");
	m_nConnectPoolMin = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/Counter_Common/connectpool/max");
	m_nConnectPoolMax = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/Counter_Common/connectpool/increase");
	m_nConnectPoolIncrease = boost::lexical_cast<int>(node.node().child_value());


	node = doc.select_single_node("/config/Counter_Common/wtfs_web");
	m_sWtfsWeb = node.node().child_value();
	node = doc.select_single_node("/config/Counter_Common/wtfs_mobile");
	m_sWtfsMobile = node.node().child_value();

	node = doc.select_single_node("/config/Counter_Common/connect_retry");
	m_nConnectRetry = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/Counter_Common/business_retry");
	m_nBusinessRetry = boost::lexical_cast<int>(node.node().child_value());



	pugi::xpath_node_set system_nodes = doc.select_nodes("/config/system");
	int i=0;
	pugi::xpath_node_set::const_iterator it_system;

	for (i=1,  it_system = system_nodes.begin(); it_system != system_nodes.end(); ++it_system, ++i)
	{
		pugi::xpath_node node = *it_system;

		BusinessSystem system;

		std::string sysid = node.node().child_value("id");

		system.id = sysid;

		TRACE("system id = %s\n", sysid.c_str());
		std::string desc = node.node().child_value("desc");

		

		// 读系统下面所有的业务类型
		//std::map<BUSI_TYPE, std::map<std::string, std::vector<Counter>> > busis; 使用BusinessSystem
		boost::format fmt("/config/system[%1%]/BusinessType");
		fmt % i;
		node_path = fmt.str();
		TRACE("查询业务类型 = %s\n", node_path.c_str());
		pugi::xpath_node_set busitype_nodes = doc.select_nodes(node_path.c_str());
		
		int j=0;
		pugi::xpath_node_set::const_iterator it_busitype;
		for (j=1, it_busitype = busitype_nodes.begin(); it_busitype != busitype_nodes.end(); ++it_busitype, ++j)
		{
			pugi::xpath_node node = *it_busitype;

			BusinessType busiType;

			std::string type = node.node().child_value("type");
			int BusiType = boost::lexical_cast<int>(type);
			TRACE("业务类型 id = %d\n", BusiType);

			int CounterType = boost::lexical_cast<int>(node.node().child_value("CounterType"));
			TRACE("柜台类型 id = %d\n", CounterType);

			int m_nConnectTimeout = boost::lexical_cast<int>(node.node().child_value("connect_timeout"));
	
			int m_nIdleTimeout = boost::lexical_cast<int>(node.node().child_value("idle_timeout"));
	
			int m_nRecvTimeout = boost::lexical_cast<int>(node.node().child_value("recv_timeout"));

			std::string gydm = node.node().child_value("gydm");
			std::string wtfs_mobile = node.node().child_value("wtfs_mobile");
			std::string wtfs_web = node.node().child_value("wtfs_web");


			// 读业务类型下面所有可用的营业部
			//std::map<std::string, std::vector<Counter>> branchs;
			
			boost::format fmt("/config/system[%1%]/BusinessType[%2%]/branch");
			fmt % i %j;
			node_path = fmt.str();
			TRACE("查询营业部 = %s\n", node_path.c_str());
			pugi::xpath_node_set branch_nodes = doc.select_nodes(node_path.c_str());
			pugi::xpath_node_set::const_iterator it_branch;
			int m=0;
			for (m=1, it_branch = branch_nodes.begin(); it_branch != branch_nodes.end(); ++it_branch, ++m)
			{
				pugi::xpath_node node = *it_branch;

				
				int enable = boost::lexical_cast<int>(node.node().child_value("enable"));
				if (enable == 0)
					continue;

				std::string branchList = node.node().child_value("branch_list");
				TRACE("营业部是否启用 = %s %d\n", branchList.c_str(), enable);

				// 读营业部下面所有可用的柜台
				std::vector<Counter> counters;
				boost::format fmt("/config/system[%1%]/BusinessType[%2%]/branch[%3%]/server");
				fmt % i % j % m;
				node_path = fmt.str();
				TRACE("查询柜台 = %s\n", node_path.c_str());
				pugi::xpath_node_set counter_nodes = doc.select_nodes(node_path.c_str());
				pugi::xpath_node_set::const_iterator it_counter;
				for (it_counter = counter_nodes.begin(); it_counter != counter_nodes.end(); ++it_counter)
				{
					pugi::xpath_node node = *it_counter;

					node_value = node.node().child_value("enable");
					int enable = boost::lexical_cast<int>(node_value);
					if (enable == 0)
						continue;

					Counter counter;

					node_value = node.node().child_value("servername");
					
					counter.m_sServerName = node_value;
					

					node_value = node.node().child_value("ip");
					TRACE("柜台 = %s\n", node_value.c_str());
					counter.m_sIP = node_value;

					node_value = node.node().child_value("port");
					counter.m_nPort = boost::lexical_cast<int>(node_value);

					node_value = node.node().child_value("username");
					counter.m_sUserName = node_value;

					node_value = node.node().child_value("password");
					counter.m_sPassword = node_value;

					node_value = node.node().child_value("req");
					counter.m_sReq = node_value;

					node_value = node.node().child_value("res");
					counter.m_sRes = node_value;

					counter.m_eCounterType = ConvertIntToCounterType(CounterType);
					counter.m_nConnectTimeout = m_nConnectTimeout;
					counter.m_nIdleTimeout = m_nIdleTimeout;
					counter.m_nRecvTimeout = m_nRecvTimeout;

					// 顶点专用
					
					counter.m_sGydm = gydm;

					// 委托方式
					
					counter.m_sWtfs_mobile = wtfs_mobile;
					counter.m_sWtfs_web = wtfs_web;

					counters.push_back(counter);///////////////////////////
				} // end for counter

				ConnectPool * pool = new ConnectPool(counters);
				busiType.connPool[branchList] = pool;//////////////////////
				busiType.counterType = ConvertIntToCounterType(CounterType);

				Branch *branch = new Branch();
				branch->servers = counters;

				busiType.branches[branchList] = branch;
			} // end for branch

			
			system.busis[ConvertIntToBusiType(BusiType)] = busiType;/////////////////////////////
		} // end for business type

		g_ConnectManager.systems[sysid] = system;
	} // end for system



	// 日志
	node = doc.select_single_node("/config/log/loglevel");
	m_nLogLevel = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/response_len");
	m_nResponseLen = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/filterfuncid");
	m_nFilterFuncId = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/filterfield");
	m_nFilterField = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/filtercustid");
	m_sFilterCustId = node.node().child_value();

	node = doc.select_single_node("/config/log/file/enable");
	m_nLogFileEnable = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/file/path");
	m_sLogFilePath = node.node().child_value();
	node = doc.select_single_node("/config/log/file/threadpool");
	m_nLogFileThreadPool = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/db/enable");
	m_nLogDbEnable = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/db/servicename");
	m_sLogDbServiceName = node.node().child_value();
	node = doc.select_single_node("/config/log/db/username");
	m_sLogDbUserName = node.node().child_value();
	node = doc.select_single_node("/config/log/db/password");
	m_sLogDbPassword = node.node().child_value();
	node = doc.select_single_node("/config/log/db/min");
	m_nLogDbMin = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/db/max");
	m_nLogDbMax = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/db/increase");
	m_nLogDbIncrease = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/db/threadpool");
	m_nLogDbThreadPool = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/gui/enable");
	m_nLogGuiEnable = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/gui/showcount");
	m_nLogGuiShowCount = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/gui/threadpool");
	m_nLogShowThreadPool = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/log/mq/enable");
	m_nLogMqEnable = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/mq/threadpool");
	m_nLogMqThreadPool = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/mq/min");
	m_nLogMqMin = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/mq/max");
	m_nLogMqMax = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/log/mq/increase");
	m_nLogMqIncrease = boost::lexical_cast<int>(node.node().child_value());

	pugi::xpath_node_set nodesMQServers = doc.select_nodes("/config/log/mq/mqserver");
	pugi::xpath_node_set::const_iterator itMQServers;

	for (itMQServers = nodesMQServers.begin(); itMQServers != nodesMQServers.end(); ++itMQServers)
	{
		pugi::xpath_node node = *itMQServers;



		std::string mqserver = node.node().child_value();
		
		m_vLogMqServer.push_back(mqserver);
		
	}


	// 监控
	node = doc.select_single_node("/config/monitor/tcp");
	m_nMonitorTcpPort = boost::lexical_cast<int>(node.node().child_value());


	// 崩溃
	node = doc.select_single_node("/config/dump/path");
	m_sDumpPath = node.node().child_value();



	node = doc.select_single_node("/config/captcha_timeout");
	captcha_timeout = boost::lexical_cast<int>(node.node().child_value());

	// OCR
	node = doc.select_single_node("/config/OCR/path");
	OCRPath = node.node().child_value();

	node = doc.select_single_node("/config/OCR/output");
	OCROutput = node.node().child_value();

	node = doc.select_single_node("/config/OCR/name/algorithm");
	m_nNameAlgorithm = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/OCR/name/threshold");
	m_nNameThreshold = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/OCR/address/algorithm");
	m_nAddressAlgorithm = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/OCR/address/threshold");
	m_nAddressThreshold = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/OCR/idno/algorithm");
	m_nIDNoAlgorithm = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/OCR/idno/threshold");
	m_nIDNoThreshold = boost::lexical_cast<int>(node.node().child_value());

	node = doc.select_single_node("/config/OCR/expire/algorithm");
	m_nExpireAlgorithm = boost::lexical_cast<int>(node.node().child_value());
	node = doc.select_single_node("/config/OCR/expire/threshold");
	m_nExpireThreshold = boost::lexical_cast<int>(node.node().child_value());

	return true;
}

void ConfigManager::LoadCrossDomain()
{
//	int length;
//	char * buffer;

	std::string sFileName = gConfigManager::instance().m_sPath + "\\crossdomain.xml";

	std::ifstream is;
	is.open(sFileName.c_str());
	std::ostringstream tmp;
	tmp << is.rdbuf();
	
	m_sFlashCrossDomain = tmp.str();

	is.close();
/*
	is.open (sFileName.c_str(), std::ios::binary );

	// get length of file:
	is.seekg (0, std::ios::end);
	length = is.tellg();
	is.seekg (0, std::ios::beg);

	// allocate memory:
	buffer = new char [length];
	memset(buffer, 0, length);

	// read data as a block:
	is.read (buffer,length);
	is.close();

	m_sFlashCrossDomain = buffer;

	delete[] buffer;
*/

}

BUSI_TYPE ConfigManager::ConvertIntToBusiType(int val)
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

COUNTER_TYPE ConfigManager::ConvertIntToCounterType(int val)
{
	switch (val)
	{
	case 1:
		return CT_HS_T2;
	case 2:
		return CT_HS_COM;
	case 3:
		return CT_JZ_WIN;
	case 4:
		return CT_JZ_LINUX;
	case 5:
		return CT_DINGDIAN;
	case 6:
		return CT_JSD;
	case 7:
		return CT_XINYI;
	default:
		return CT_UNKNOWN;
	}
}

COUNTER_TYPE ConfigManager::GetCounterType(std::string SystemNo, std::string busiType)
{
	std::map<std::string, BusinessSystem>::iterator it;
	it = g_ConnectManager.systems.find(SystemNo);
	if (it == g_ConnectManager.systems.end())
	{
		return CT_UNKNOWN;
	}

	BusinessSystem& bs = it->second;

	BusiType bt = ConvertIntToBusiType( boost::lexical_cast<int>(busiType) );

	std::map<BUSI_TYPE, BusinessType >::iterator it2;
	it2 = bs.busis.find(bt);
	if (it2 == bs.busis.end())
	{
		return CT_UNKNOWN;
	}

	CounterType ct = it2->second.counterType;
	return ct;
}
