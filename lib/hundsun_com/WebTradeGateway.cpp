#include "StdAfx.h"
#include "WebTradeGateway.h"




#include "ConnectPool.h"

#include "tradegatewaygtjadlg.h"
#include "tradegatewaygtja.h"
#include "ConfigManager.h"


CWebTradeGateway::CWebTradeGateway(CConnect* pConn)
{
	m_pConn = pConn;
	m_nReTry = g_ConfigManager.m_nRetry;
}


CWebTradeGateway::~CWebTradeGateway()
{
}


void CWebTradeGateway::SendRequest(CString sRequest, CString& sResponse)
{
	sResponse.Empty();
	
	
	CString strFuncId;
	CString strBranchNo;
	int nRet = 0;	

	int i = 0;
	CString strNameValue;
	CString strName, strValue;

	while (AfxExtractSubString(strNameValue, sRequest, i))
	{
		i++;

	   if (!AfxExtractSubString(strName, strNameValue, 0, '='))
	   {
		  continue;
	   }

	   if (!AfxExtractSubString(strValue, strNameValue, 1, '='))
	   {
		  continue;
	   }

	   if (strName == "function_id")
	   {
			strFuncId = strValue;
	   }

	   if (strName == "op_branch_no")
	   {
		   strBranchNo = strValue;
	   }
	}

	//TRACE("营业部=%s\n", strBranchNo);
	//TRACE("功能号=%s\n", strFuncId);
	int lBranchNo = atoi(strBranchNo);
	int lFuncNo = atoi(strFuncId);

	m_pConn->lpConnection->SetHead(lBranchNo, lFuncNo);
	
	i = i - 1;
	//TRACE("设置行列数，行数＝%d\n", i);
	m_pConn->lpConnection->SetRange(i, 1);

	i = 0;
	while (AfxExtractSubString(strNameValue, sRequest, i))
	{
	   i++;
//	   CString strName, strValue;

	   if (!AfxExtractSubString(strName, strNameValue, 0, '='))
	   {
		  continue;
	   }

	   if (!AfxExtractSubString(strValue, strNameValue, 1, '='))
	   {
		  continue;
	   }

//		TRACE("AddField=%s\n", strName);
		m_pConn->lpConnection->AddField(strName);
	}

	i = 0;
	while (AfxExtractSubString(strNameValue, sRequest, i))
	{
	   i++;
	   //CString strName, strValue;

	   if (!AfxExtractSubString(strName, strNameValue, 0, '='))
	   {
		  continue;
	   }

	   if (!AfxExtractSubString(strValue, strNameValue, 1, '='))
	   {
		  continue;
	   }

//		TRACE("AddValue=%s\n", strValue);
		m_pConn->lpConnection->AddValue(strValue);
	}

	int nSenderId = 1;
	
	m_pConn->lpConnection->put_SenderId(nSenderId); // 此处是不是可以改成客户号之类唯一代表客户的值


	nRet = m_pConn->lpConnection->Send();
	if (nRet != 0)
	{
		// 发送失败
		sResponse = "cssweb_code=error&cssweb_msg=发送数据失败\n";
		return;
	}

	m_pConn->lpConnection->FreePack();

	nRet = m_pConn->lpConnection->Receive();
	if (nRet != 0)
	{
		// 接收失败
		sResponse = "cssweb_code=error&cssweb_msg=接收数据失败\n";
		return;
	}

	long nRecvSenderId = m_pConn->lpConnection->get_SenderId();
	if (nRecvSenderId != nSenderId)
	{
		sResponse = "cssweb_code=error&cssweb_msg=发送和接收不匹配\n";
		return;
	}

	
	while (m_pConn->lpConnection->get_Eof() == 0)
	{
		int fieldCount = m_pConn->lpConnection->get_FieldCount();
		
		for (int i=0; i<fieldCount; i++)
		{
				CString fieldName = m_pConn->lpConnection->GetFieldName(i);
				CString fieldValue = m_pConn->lpConnection->FieldByName(fieldName);
				
				CString sKeyValue = fieldName + "=" + fieldValue;
				//TRACE("%s\n", sKeyValue);

				if (i != (fieldCount-1))
				{
					//sResponse.Append(sKeyValue + "&");
					sResponse = sResponse + sKeyValue + "&";
				}
				else
				{
					//sResponse.Append(sKeyValue);
					sResponse = sResponse + sKeyValue;
				}
		}

		sResponse = sResponse + "\n";
		m_pConn->lpConnection->MoveBy(1);
	}

	if (sResponse.GetLength() <= 0)
	{
		sResponse = "cssweb_code=success&cssweb_msg=没有数据\n";
	}
}

