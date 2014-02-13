#include "DistributedLogManager.h"

#include "./config/ConfigManager.h"

#include "./output/FileLog.h"

// xml
#include "./lib/xml/rapidxml.hpp"
#include "./lib/xml/rapidxml_utils.hpp"

#include "log/mq/LogConnect.h"
#include "log/mq/LogConnectPool.h"


DistributedLogManager::DistributedLogManager(void)
	:kafka_worker_(kafka_q_, boost::bind(&DistributedLogManager::kafka_log, this, _1), gConfigManager::instance().m_nLogMqThreadPool)
{
	logFileName = "分布式日志";
}


DistributedLogManager::~DistributedLogManager(void)
{
}


void DistributedLogManager::start()
{
	
	kafka_worker_.start();
	
}

void DistributedLogManager::stop()
{
	
	kafka_worker_.stop();
	
}

void DistributedLogManager::push(Trade::TradeLog log)
{
	
	if (gConfigManager::instance().m_nLogMqEnable)
	{
		
		kafka_q_.push(log);
	}
	
}

bool DistributedLogManager::kafka_log(Trade::TradeLog log)
{
/*
{
  "logType":"trade",  //日志类型。'trade':交易日志
  "message":{
    "logLevel":"warn", //日志级别
    "request":"", //交易请求。
    "requestTime":"2012-05-10 13:00:00", //请求时间点
    "requestRuntime":"", //请求耗时。单位：毫秒
    "status":"0", //交易状态。1：成功；0：失败
    "response":"", //交易请求响应。
    "sourceSysNo":"njzq_jlp", //交易请求来源系统编号
    "sourceSysVer":"iphone", //交易请求来源系统版本
    "counterIp":"", //柜台IP
    "counterPort":"", //柜台端口
    "gatewayIp":"", //交易网关IP
    "gatewayPort":"" //交易网关端口
  }
}
*/

	std::string json;
	json = "{";

	std::string SOH = "\x01";

	

	std::string funcid = log.funcid();
	// 心跳不记录
	if (funcid == "999999")
	{
	//	delete log;
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
					//delete log;
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
					//delete log;
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
					delete this;
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

	json += "\"request\":\"";
	json += sFilterRequest;
	json += "\",";


	std::string sysNo = log.sysno();
	if (sysNo.empty())
		sysNo = "no_sysno";
	json += "\"sourceSysNo\":\"";
	json += sysNo;
	json += "\",";

	std::string sysVer = log.sysver();
	if (sysVer.empty())
		sysVer = "no_sysver";
	json += "\"sourceSysVer\":\"";
	json += sysVer;
	json += "\",";

	std::string busiType = log.busitype();
	if (busiType.empty())
		busiType = "no_busitype";
	json += "\"busiType\":\"";
	json += busiType;
	json += "\",";


	// 账户和日志级别
	//std::string account = logMsg.account();
	//if (account.empty())
	//	sLogFileName += "no_account";
	//else
	//	sLogFileName += account;

	json += "\"logLevel\":\"";
	switch (log.level())
	{
	case Trade::TradeLog::DEBUG_LEVEL:
		json += "debug";
		break;
	case Trade::TradeLog::INFO_LEVEL:
		json += "info";
		break;
	case Trade::TradeLog::WARN_LEVEL:
		json += "warn";
		break;
	case Trade::TradeLog::ERROR_LEVEL:
		json += "error";
		break;
	default:
		json += "error";
	}
	json += "\",";


	json += "\"requestTime\":\"";
	json += log.recvtime();
	json += "\",";

		
	json += "\"requestRuntime\":\"";
	json += boost::lexical_cast<std::string>(log.runtime()/1000);
	json += "\",";

	json += "\"status\":\"";
	json += boost::lexical_cast<std::string>(log.status());
	json += "\",";
	
	json += "\"response\":\"";
	json += log.response();
	json += "\",";

	json += "\"counterIp\":\"";
	json += log.gtip();
	json += "\",";

	json += "\"counterPort\":\"";
	json += log.gtport();
	json += "\",";

	json += "\"gatewayIp\":\"";
	json += log.gatewayip();
	json += "\",";

	json += "\"gatewayPort\":\"";
	json += log.gatewayport();
	json += "\""; // 注意：最后没有逗号

	json += "}";


	LogConnect * pConnect = gLogConnectPool::instance().GetConnect();
	if (pConnect == NULL)
	{
		gFileLog::instance().Log(json, "写分布式日志失败");
		
	}
	else
	{
		if (pConnect->IsConnected())
		{
			std::string response = "";
			if (pConnect->Send(json, response))
			{
				// 归还连接
				gLogConnectPool::instance().PushConnect(pConnect);
			
			}
			else
			{
				delete pConnect; // 不归还连接，需要释放
				gFileLog::instance().Log(json, "写分布式日志失败");
			}
		}
		else
		{
			if (pConnect->ReConnect())
			{
				std::string response = "";
				if (pConnect->Send(json, response))
				{
					// 归还连接
					gLogConnectPool::instance().PushConnect(pConnect);
				}
				else
				{
					delete pConnect; // 不归还连接，需要释放
					gFileLog::instance().Log(json, "写分布式日志失败");
				}
			}
			else
			{
				delete pConnect; // 不归还连接，需要释放
				gFileLog::instance().Log(json, "写分布式日志失败");
			}
		}
	} // end if

	

	// 释放
	//log.destroy();
	//delete log;

	return true;
}

void DistributedLogManager::GetFilterMap(std::string& request, std::map<std::string, std::string>& mapFieldFilter, std::map<std::string, std::string>& reqmap)
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


void DistributedLogManager::LoadFieldFilter()
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


void DistributedLogManager::LoadFuncFilter()
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
void DistributedLogManager::LoadFieldFilter(std::string& sFieldFilterXML, std::map<std::string, std::string>& mapFieldFilter)
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
void DistributedLogManager::LoadFuncFilter(std::string& sFuncFilterXML, std::map<std::string, FUNCTION_DESC>& mapFuncFilter)
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
