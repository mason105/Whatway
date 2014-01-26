#include "StdAfx.h"

#include <strstream>

#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "DingDian.h"



#include "./lib/xml/rapidxml.hpp"
#include "./lib/xml/rapidxml_utils.hpp"
#include "./config/ConfigManager.h"


#include "./lib/apexsoft/fiddef.h"
#include "./lib/apexsoft/fixapi.h"


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

	/*

	std::string sDLLFileName = gConfigManager::instance().m_sPath + "\\fixapi.dll";
	hFixAPI = LoadLibrary(sDLLFileName.c_str());
	if (hFixAPI == NULL)
	{
		AfxMessageBox("load fixapi.dll error.");
		return FALSE;
	}

		 

	


	Fix_Encode = (fpFix_Encode) GetProcAddress(hFixAPI, "Fix_Encode");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Encode error.");
		goto FREE_DLL;
	}

	Fix_GetCount = (fpFix_GetCount) GetProcAddress(hFixAPI, "Fix_GetCount");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_GetCount error.");
		goto FREE_DLL;
	}

	Fix_GetErrMsg = (fpFix_GetErrMsg) GetProcAddress(hFixAPI, "Fix_GetErrMsg");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_GetErrMsg error.");
		goto FREE_DLL;
	}

	Fix_GetCode = (fpFix_GetCode) GetProcAddress(hFixAPI, "Fix_GetCode");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_GetCode error.");
		goto FREE_DLL;
	}

	Fix_Run = (fpFix_Run) GetProcAddress(hFixAPI, "Fix_Run");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Run error.");
		goto FREE_DLL;
	}

	Fix_CreateHead = (fpFix_CreateHead) GetProcAddress(hFixAPI, "Fix_CreateHead");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_CreateHead error.");
		goto FREE_DLL;
	}

	Fix_SetGYDM = (fpFix_SetGYDM) GetProcAddress(hFixAPI, "Fix_SetGYDM");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetGYDM error.");
		goto FREE_DLL;
	}
	
	Fix_SetNode = (fpFix_SetNode) GetProcAddress(hFixAPI, "Fix_SetNode");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetNode error.");
		goto FREE_DLL;
	}

	Fix_SetDestFBDM = (fpFix_SetDestFBDM) GetProcAddress(hFixAPI, "Fix_SetDestFBDM");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetDestFBDM error.");
		goto FREE_DLL;
	}
	 
	Fix_SetFBDM = (fpFix_SetFBDM) GetProcAddress(hFixAPI, "Fix_SetFBDM");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetFBDM error.");
		goto FREE_DLL;
	}

	Fix_SetWTFS = (fpFix_SetWTFS) GetProcAddress(hFixAPI, "Fix_SetWTFS");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetWTFS error.");
		goto FREE_DLL;
	}

	Fix_SetTimeOut = (fpFix_SetTimeOut) GetProcAddress(hFixAPI, "Fix_SetTimeOut");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetTimeOut error.");
		goto FREE_DLL;
	}

	Fix_ReleaseSession = (fpFix_ReleaseSession) GetProcAddress(hFixAPI, "Fix_ReleaseSession");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_ReleaseSession error.");
		goto FREE_DLL;
	}
	
	Fix_AllocateSession = (fpFix_AllocateSession) GetProcAddress(hFixAPI, "Fix_AllocateSession");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_AllocateSession error.");
		goto FREE_DLL;
	}
	
	Fix_Close = (fpFix_Close) GetProcAddress(hFixAPI, "Fix_Close");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Close error.");
		goto FREE_DLL;
	}

	Fix_Connect = (fpFix_Connect) GetProcAddress(hFixAPI, "Fix_Connect");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Connect error.");
		goto FREE_DLL;
	}

	Fix_Uninitialize = (fpFix_Uninitialize) GetProcAddress(hFixAPI, "Fix_Uninitialize");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Uninitialize error.");
		goto FREE_DLL;
	}

	Fix_Initialize = (fpFix_Initialize) GetProcAddress(hFixAPI, "Fix_Initialize");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_Initialize error.");
		goto FREE_DLL;
	}

	Fix_SetItem = (fpFix_SetItem) GetProcAddress(hFixAPI, "Fix_SetItem");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_SetItem error.");
		goto FREE_DLL;
	}

	Fix_GetItem = (fpFix_GetItem) GetProcAddress(hFixAPI, "Fix_GetItem");
	if (Fix_Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Fix_GetItem error.");
		goto FREE_DLL;
	}
	

#ifdef _USE_DINGDIAN_
	Fix_Initialize();
#endif
	
	gFileLog::instance().Log("初始化顶点柜台成功");

	
	return TRUE;

FREE_DLL:
	if (hFixAPI != NULL)
		FreeLibrary(hFixAPI);
	return FALSE;
	*/

		gFileLog::instance().Log("开始初始化顶点柜台");
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
