#include "FileLogManager.h"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

// xml
#include "./lib/xml/rapidxml.hpp"
#include "./lib/xml/rapidxml_utils.hpp"

#include "./config/ConfigManager.h"
#include "./output/FileLog.h"




FileLogManager::FileLogManager(void)
	:file_worker_(file_q_, boost::bind(&FileLogManager::file_log, this, _1), gConfigManager::instance().m_nLogFileThreadPool)
{
}


FileLogManager::~FileLogManager(void)
{
}

void FileLogManager::start()
{
	
	file_worker_.start();
	
}

void FileLogManager::stop()
{
	
	file_worker_.stop();
	
}

// 文件日志线程池处理函数
bool FileLogManager::file_log(Trade::TradeLog log)
{
	std::string SOH = "\x01";

	

	std::string funcid = log.funcid();

	// 心跳不记录
	if (funcid == "999999")
	{
		
		return true;
	}

	std::string countertype = log.countertype();

	// 恒生t2
	if (countertype == "1")
	{
		// 如果启用了过滤查询功能号
		if (gConfigManager::instance().m_nFilterFuncId)
		{
			std::map<std::string, FUNCTION_DESC>::iterator it = m_mT2_FilterFunc.find(funcid);
			if (it != m_mT2_FilterFunc.end())
			{
				// 过滤查询功能号
				if (it->second.isQuery)
				{
					//log.destroy();
					
					return true;
				}
			}
		}
	}
	
	// 金证
	if (countertype == "3")
	{
		// 如果启用了过滤查询功能号
		if (gConfigManager::instance().m_nFilterFuncId)
		{
			std::map<std::string, FUNCTION_DESC>::iterator it = m_mKingdom_FilterFunc.find(funcid);
			if (it != m_mKingdom_FilterFunc.end())
			{
				// 过滤查询功能号
				if (it->second.isQuery)
				{
					//log.destroy();
					
					return true;
				}
			}
		}
	}

	// 顶点
	if (countertype == "5")
	{
		// 如果启用了过滤查询功能号
		if (gConfigManager::instance().m_nFilterFuncId)
		{
			std::map<std::string, FUNCTION_DESC>::iterator it = m_mDingDian_FilterFunc.find(funcid);
			if (it != m_mDingDian_FilterFunc.end())
			{
				// 过滤查询功能号
				if (it->second.isQuery)
				{
					//log.destroy();
					
					return true;
				}
			}
		}
	}

	std::string request = log.request();
	std::map<std::string, std::string> reqmap;

	// 过滤字段
	if (countertype == "1")
	{
		GetFilterMap(request, m_mT2_FilterField, reqmap);
	}
	else if (countertype == "3")
	{
		GetFilterMap(request, m_mKingdom_FilterField, reqmap);
	}
	else if (countertype == "5")
	{
		GetFilterMap(request, m_mDingDian_FilterField, reqmap);
	}
	else if (countertype == "6") // agc
	{
		GetFilterMap(request, m_mAGC_FilterField, reqmap);
	}
	else if (countertype == "7") // xinyi
	{
		GetFilterMap(request, m_mXinyi_FilterField, reqmap);
	}

	

	std::string sFilterRequest = "";
	for (std::map<std::string, std::string>::iterator it = reqmap.begin(); it != reqmap.end(); it++)
	{
		sFilterRequest += it->first;
		sFilterRequest += "=";
		sFilterRequest += it->second;
		sFilterRequest += SOH;
	}

	if (sFilterRequest.empty())
		sFilterRequest = request;


	std::string sLogFileName = gConfigManager::instance().m_sLogFilePath;
	sLogFileName += "\\";

	std::string sysNo = log.sysno();
	if (sysNo.empty())
		sysNo = "no_sysno";
	sLogFileName += sysNo;
	sLogFileName += "\\";

	std::string sysVer = log.sysver();
	if (sysVer.empty())
		sysVer = "no_sysver";
	sLogFileName += sysVer;
	sLogFileName += "\\";

	std::string busiType = log.busitype();
	if (busiType.empty())
		busiType = "no_busitype";
	sLogFileName += busiType;
	sLogFileName += "\\";


	// 当前日期
	boost::gregorian::date day = boost::gregorian::day_clock::local_day();
	sLogFileName += to_iso_extended_string(day);
	sLogFileName += "\\";



	// 创建目录
	boost::filesystem::path p(sLogFileName);
	if (!boost::filesystem::exists(p))
	{
		boost::filesystem::create_directories(p);
	}

	// 账户和日志级别
	std::string account = log.account();
	if (account.empty())
		sLogFileName += "no_account";
	else
		sLogFileName += account;

	switch (log.level())
	{
	case Trade::TradeLog::DEBUG_LEVEL:
		sLogFileName += "_debug.log";
		break;
	case Trade::TradeLog::INFO_LEVEL:
		sLogFileName += "_info.log";
		break;
	case Trade::TradeLog::WARN_LEVEL:
		sLogFileName += "_warn.log";
		break;
	case Trade::TradeLog::ERROR_LEVEL:
		sLogFileName += "_error.log";
		break;
	default:
		sLogFileName += "_nolevel.log";
	}



	std::ofstream outfile(sLogFileName.c_str(), std::ios_base::app);
	if (outfile.is_open())
	{
		outfile << "功能号: " << funcid << "\n";
		outfile << "客户端IP：" << log.ip() << "\n";

		outfile << "请求接收时间: " << log.recvtime() << "\n";
		outfile << "应答发送时间：" << log.sendtime() << "\n";

		outfile << "请求开始处理时间：" << log.begintime() << "\n";
		outfile << "执行时间(毫秒)：" << log.runtime()/1000 << "\n";
		
		outfile << "柜台地址：" << log.gtip()<< "\n";
		outfile << "柜台端口：" << log.gtport()<< "\n";
		

		outfile << "请求密文：" << log.enc_request() << "\n";
		// 需要过滤敏感字段
		outfile << "请求过滤后明文：" <<  sFilterRequest << "\n";


		outfile << "执行结果：" << log.status() << "\n";
		outfile << "柜台返回码：" << log.retcode() << "\n";
		outfile << "柜台返回消息：" << log.retmsg() << "\n";

		// 需要过滤敏感字段
		outfile << "应答明文：" << log.response() << "\n";
		outfile << "应答密文：" << log.enc_response() << "\n";
			
		// 隔一行
		outfile << "\n";

		outfile.close();
	}
	else
	{
		std::string sLog = "打开日志文件" + sLogFileName + "失败.";
		gFileLog::instance().Log(sLog);
	}

	// 释放
	//log.destroy();
	

	return true;
}

void FileLogManager::push(Trade::TradeLog logMsg)
{

		file_q_.push(logMsg);
	
}

/*
根据过滤字段定义mapFieldFilter，过滤请求request，返回过滤后结果reqmap
*/
void FileLogManager::GetFilterMap(std::string& request, std::map<std::string, std::string>& mapFieldFilter, std::map<std::string, std::string>& reqmap)
{
	std::string SOH = "\x01";

	std::vector<std::string> keyvalues;
	boost::split(keyvalues, request, boost::is_any_of(SOH)); // 注意需要通过配置文件配置

	
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

		// 如果启用了过滤关键字段
		if (gConfigManager::instance().m_nFilterField)
		{
			if (mapFieldFilter.find(key) != mapFieldFilter.end())
				continue;
		}



		std::string value = "";
		if (!kv[1].empty())
			value = kv[1];

		reqmap[key] = value;
	}
}

// 读过滤字段的配置文件
void FileLogManager::LoadFieldFilter()
{
	std::string xmlfile = "";
	
	// 顶点
	xmlfile = gConfigManager::instance().m_sPath + "\\apex_filterfield.xml";
	LoadFieldFilter(xmlfile, m_mDingDian_FilterField);

	// 恒生T2
	xmlfile = gConfigManager::instance().m_sPath + "\\hundsun_t2_filterfield.xml";
	LoadFieldFilter(xmlfile, m_mT2_FilterField);

	// 金证
	xmlfile = gConfigManager::instance().m_sPath + "\\kingdom_win_filterfield.xml";
	LoadFieldFilter(xmlfile, m_mKingdom_FilterField);

}

// 读过滤查询功能号的配置文件
void FileLogManager::LoadFuncFilter()
{
	std::string xmlfile = "";
	
	// 顶点
	xmlfile = gConfigManager::instance().m_sPath + "\\apex_filterfunc.xml";
	LoadFuncFilter(xmlfile, m_mDingDian_FilterFunc);

	// 恒生T2
	xmlfile = gConfigManager::instance().m_sPath + "\\hundsun_t2_filterfunc.xml";
	LoadFuncFilter(xmlfile, m_mT2_FilterFunc);

	// 金证
	xmlfile = gConfigManager::instance().m_sPath + "\\kingdom_win_filterfunc.xml";
	LoadFuncFilter(xmlfile, m_mKingdom_FilterFunc);
}

// 过滤敏感字段
void FileLogManager::LoadFieldFilter(std::string& sFieldFilterXML, std::map<std::string, std::string>& mapFieldFilter)
{
	rapidxml::file<> f(sFieldFilterXML.c_str());

	rapidxml::xml_document<> doc;

	doc.parse<0>(f.data());

	rapidxml::xml_node<> * root = doc.first_node();


	

	rapidxml::xml_node<> * filter = root->first_node();
	while (filter != NULL)
	{
		//TRACE("name1 = %s\n", filter->name());

		rapidxml::xml_node<>* fields = filter->first_node();
		while (fields != NULL)
		{
			//TRACE("\tname2 = %s\n", fields->name());

			rapidxml::xml_node<>* field = fields->first_node();
			while (field != NULL)
			{
				//TRACE("\t\tname3 = %s\n", field->name());

				std::string sFieldName = field->value();
				mapFieldFilter[sFieldName] = ""; // value保留，待以后扩展用

				field = field->next_sibling();
			}

			fields = fields->next_sibling();
		} // end while

		filter = filter->next_sibling();
	} // end while
}

// 过滤查询类功能号
void FileLogManager::LoadFuncFilter(std::string& sFuncFilterXML, std::map<std::string, FUNCTION_DESC>& mapFuncFilter)
{
	rapidxml::file<> f(sFuncFilterXML.c_str());

	rapidxml::xml_document<> doc;

	doc.parse<0>(f.data());

	rapidxml::xml_node<> * root = doc.first_node();

	
	

	rapidxml::xml_node<> * filter = root->first_node();
	while (filter != NULL)
	{
		//TRACE("name1 = %s\n", filter->name());
		

		rapidxml::xml_node<> * functions = filter->first_node();
		while (functions != NULL)
		{
			//TRACE("\tname2 = %s\n", functions->name());
			if (strcmp(functions->name(), "function") == 0)
			{

			//rapidxml::xml_node<> * function = functions->first_node();
			//while (function != NULL)
			//{
			//	TRACE("\t\tname3 = %s\n", function->name());

				FUNCTION_DESC funcDesc;

				std::string sFuncId = functions->first_attribute("id")->value();
				std::string sIsQuery = functions->first_attribute("isQuery")->value();
				std::string sHasResultSet = functions->first_attribute("hasResultSet")->value();
			
				if (sIsQuery == "true")
					funcDesc.isQuery = true;
				else
					funcDesc.isQuery = false;

				if (sHasResultSet == "true")
					funcDesc.hasResultRet = true;
				else
					funcDesc.hasResultRet = false;

				mapFuncFilter[sFuncId] = funcDesc;
			}

			//	function = function->next_sibling();
			//}

			functions = functions->next_sibling();
		}

		filter = filter->next_sibling();
	} // end while
}
