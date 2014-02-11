#include "StdAfx.h"

#include <strstream>

#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "DingDian.h"



#include "./lib/xml/rapidxml.hpp"
#include "./lib/xml/rapidxml_utils.hpp"
#include "./config/ConfigManager.h"


#include "business/apexsoft/fiddef.h"
#include "business/apexsoft/fixapi.h"


#include "./output/FileLog.h"


CDingDian g_DingDian;

CDingDian::CDingDian(void)
{
	m_bIsInited = FALSE;
}

CDingDian::~CDingDian(void)
{
}

BOOL CDingDian::Init()
{	
	// 已经初始化过，不允许再次初始化
	if (m_bIsInited)
		return FALSE;

	Fix_Initialize();
	m_bIsInited = TRUE;

	return TRUE;
}

void CDingDian::UnInit()
{
	Fix_Uninitialize();
}

void CDingDian::ReadMapFile()
{
	//boost::format fmt("%s\n");

	//std::string test = fmt % "ss";
	std::string mapfile = gConfigManager::instance().m_sPath + "\\tradeparam.map";

	std::ifstream f;
	f.open(mapfile.c_str());

	char szLine[256];
	std::string sLine;
	std::string key;
	std::string value;

	while ( f.getline(szLine, sizeof(szLine)) )
	{
		/*
		sLine = szLine;
		//TRACE("%s\n", sLine.c_str());


		key = sLine.substr(0,23);
		boost::algorithm::trim_right(key);
		//TRACE("字段名%s, 长度%d\n", key.c_str(), key.length());


		value = sLine.substr(23, 4);
		boost::algorithm::trim_right(value);
		//TRACE("字段值%s, 长度%d\n", value.c_str(), value.length());

		m_mRequestField[key] = boost::lexical_cast<long>(value);
		*/
		//---------------------------------------------------------
		sLine = szLine;

		std::stringstream tempStr(sLine);
		tempStr >> key ;
		long tempValue = 0;
		tempStr >> tempValue ;

		m_mRequestField[key] = tempValue;
		
	}

	f.close();

	for (std::map<std::string, long>::iterator it = m_mRequestField.begin(); it != m_mRequestField.end(); it++)
	{
		//TRACE("%s=%ld\n", it->first.c_str(), it->second);
	}
	
}

void CDingDian::ReadOutDomainFile()
{
	std::string xmlfile = gConfigManager::instance().m_sPath + "\\outputdomain.xml";

	rapidxml::file<> f(xmlfile.c_str());

	rapidxml::xml_document<> doc;

	doc.parse<0>(f.data());

	rapidxml::xml_node<> * root = doc.first_node();


	//std::cout << "name = " << root->name() << std::endl;

	rapidxml::xml_node<> * entity = root->first_node();
	while (entity != NULL)
	{
		std::string sFuncId = entity->first_attribute("FUNCTION")->value();

		std::map<std::string, long> field;
		rapidxml::xml_node<>* item = entity->first_node();
		while (item != NULL)
		{
/*			for (xml_attribute<> *attr = node11->first_attribute();
				attr; attr = attr->next_attribute())
			{
				std::cout<< attr->name() << " "<< attr->value() << std::endl;
			}
*/
			std::string sItemName = item->first_attribute("NAME")->value();
			std::string sItemValue = item->first_attribute("VALUE")->value();

			field[sItemName] = boost::lexical_cast<long>(sItemValue);

			item = item->next_sibling();
		}

		m_mReturnField[sFuncId] = field;

		entity = entity->next_sibling();
	}


	return;
}
