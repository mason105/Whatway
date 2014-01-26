#include "stdafx.h"

#include "AGC.h"
#include "./config/ConfigManager.h"

#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"


#include "./lib/xml/pugixml.hpp"
#include "./lib/xml/rapidxml.hpp"

#include "sywg.h"


AGC gAGC;

AGC::AGC(void)
{
}


AGC::~AGC(void)
{
}

bool AGC::LoadXML()
{
	bool bRet = false;

	std::string xml = gConfigManager::instance().m_sPath + "\\agc.xml";

	pugi::xml_document doc;
	if (!doc.load_file(xml.c_str()))
		return false;
	
	pugi::xpath_node_set nodesFunc = doc.select_nodes("/functions/function");
	pugi::xpath_node_set::const_iterator itFunc;
	int i=0;
	

	for (i=1,  itFunc = nodesFunc.begin(); itFunc != nodesFunc.end(); itFunc++, i++)
	{
		pugi::xpath_node node = *itFunc;

		FUNC func;

		std::string funcid = node.node().attribute("funcid").value();
		func.FuncId = funcid;

		std::string response = node.node().attribute("response").value();
		func.response = response;
	

		// request
		boost::format fmtRequest("/functions/function[%1%]/request/field");
		fmtRequest % i;
		std::string pathRequest = fmtRequest.str();

		int sizeRequest = 0;

		std::vector<FIELD> vRequest;
		pugi::xpath_node_set nodesRequest = doc.select_nodes(pathRequest.c_str());
		for (pugi::xpath_node_set::const_iterator itRequest = nodesRequest.begin(); itRequest != nodesRequest.end(); itRequest++)
		{
			pugi::xpath_node node = *itRequest;

			FIELD field;

			std::string name = node.node().attribute("name").value();
			field.name = name;

			std::string type = node.node().attribute("type").value();
			field.type = type;

			int size = node.node().attribute("size").as_int();
			field.size = size;

			bool encrypt = node.node().attribute("encrypt").as_bool();
			field.bEncrypt = encrypt;

			
			if (type == "char")
			{
				sizeRequest += field.size;
			}
			else
			{
				sizeRequest += GetSizeByType(type);
			}

			vRequest.push_back(field);
		}
		func.vRequest = vRequest;
		func.sizeRequest = sizeRequest;
		
		// result
		boost::format fmtResult("/functions/function[%1%]/result/field");
		fmtResult % i;
		std::string pathResult = fmtResult.str();

		int sizeResult = 0;

		std::vector<FIELD> vResult;
		pugi::xpath_node_set nodesResult = doc.select_nodes(pathResult.c_str());
		for (pugi::xpath_node_set::const_iterator itResult = nodesResult.begin(); itResult != nodesResult.end(); itResult++)
		{
			pugi::xpath_node node = *itResult;

			FIELD field;

			std::string name = node.node().attribute("name").value();
			field.name = name;

			std::string type = node.node().attribute("type").value();
			field.type = type;

			int size = node.node().attribute("size").as_int();
			field.size = size;

			if (type == "char" )
			{
				sizeResult += field.size;
			}
			else
			{
				sizeResult += GetSizeByType(type);
			}

			vResult.push_back(field);
		}
		func.vResult = vResult;
		func.sizeResult = sizeResult;

		// return
		boost::format fmtReturn("/functions/function[%1%]/return/field");
		fmtReturn % i;
		std::string pathReturn = fmtReturn.str();

		int sizeReturn = 0;

		std::vector<FIELD> vReturn;
		pugi::xpath_node_set nodesReturn = doc.select_nodes(pathReturn.c_str());
		for (pugi::xpath_node_set::const_iterator itReturn = nodesReturn.begin(); itReturn != nodesReturn.end(); itReturn++)
		{
			pugi::xpath_node node = *itReturn;

			FIELD field;

			std::string name = node.node().attribute("name").value();
			field.name = name;

			std::string type = node.node().attribute("type").value();
			field.type = type;

			int size = node.node().attribute("size").as_int();
			field.size = size;

			if (type == "char" )
			{
				sizeReturn += field.size;
			}
			else
			{
				sizeReturn += GetSizeByType(type);
			}

			vReturn.push_back(field);
		}
		func.vReturn = vReturn;
		func.sizeReturn = sizeReturn;

		m_Funcs[func.FuncId] = func;
	}
	
	

	return bRet;
}

int AGC::GetSizeByType(std::string type)
{
	int size = 0;

	if (type == "WORD")
	{
		
		size = sizeof(unsigned short);
	}

	if (type == "long")
	{
		size = sizeof(long);
	}

	if (type == "BYTE")
	{
		size = sizeof(BYTE);
	}

	if (type == "int")
	{
		size = sizeof(int);
	}

	if (type == "SWIMoney")
	{
		size = sizeof(SWIMoney);
	}

	if (type == "short")
	{
		size = sizeof(short);
	}

	if (type == "SWISmallMoney")
	{
		size = sizeof(long);
	}


	return size;
}

FUNC AGC::GetFunctionById(std::string FuncId)
{
	return m_Funcs[FuncId];
}

// 查找功能是否已定义
bool AGC::FindFunctionById(std::string FuncId)
{
	std::map<std::string, FUNC>::iterator it = m_Funcs.find(FuncId);
	if (it != m_Funcs.end())
		return true;

	return false;
}
