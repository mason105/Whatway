#include "StdAfx.h"
#include "FlashTradeGateway.h"
#include "ConnectPool.h"
#include "Log.h"
#include "base64.h"
#include "aes.h"
#include "tradegatewaygtjadlg.h"
#include "tradegatewaygtja.h"
#include "ConfigManager.h"

#define REQ_BUF_SIZE 4096
#define RES_BUF_SIZE 8192

CFlashTradeGateway::CFlashTradeGateway(CConnect* pConn)
{
	k2base64 = "vdVVTto7qp+jqA8SbIcZKDJQeLsPmtegbYuizX+LBHJlIcBvo94hurvaNgw6wLZdUtXkSGOSiJA6Opr+fN4pgA==";

	m_pConn = pConn;

	m_nReTry = g_ConfigManager.m_nRetry;
}


CFlashTradeGateway::~CFlashTradeGateway()
{
}

// 用k2的前32个字节解密request，返回到result
bool CFlashTradeGateway::GetDecryptRequest(CString& request)
{
	int unbase64ReqLen = request.GetLength();
	
	char * unbase64Req = (char *) malloc(unbase64ReqLen); 
	memset(unbase64Req, 0x00, unbase64ReqLen);

	DecodeBase64((unsigned char*)(LPCTSTR)request, unbase64ReqLen, (unsigned char*)unbase64Req, &unbase64ReqLen);

	request = unbase64Req;

	free(unbase64Req);

	return true;
}

CString CFlashTradeGateway::GetTradePWD(CString sEncPwd)
{
	// 因为base64有可能产生=，会打乱原先的格式
	sEncPwd.Replace("$", "=");

	CString sPwd;

	CString pubKey = "23dpasd23d-23l;df9302hzv/3lvjh*5";
	int nRet = 0;

	//CString K2Base64 = "kuK1hp3C7pnOqDUc+31DPwhCO5ZZzXvy4OTNg4hn+G9W4o/ShpHFK4qvUtLK5NPZ5twnV4VjwNlKeFRtm5XLZQ==";
	
	//解码
	char * K2AesEnc = (char *) malloc(sEncPwd.GetLength()); 
	memset(K2AesEnc, 0x00, sEncPwd.GetLength());

	int K2AesEncLen = sEncPwd.GetLength();
	DecodeBase64((unsigned char*)(LPCTSTR)sEncPwd, sEncPwd.GetLength(), (unsigned char*)K2AesEnc, &K2AesEncLen);

	//解密
	char K2AesDec[256];
	memset(K2AesDec, 0x00, sizeof(K2AesDec));

	
	int K2AesDecLen = K2AesEncLen;

	nRet = AESDecode((unsigned char*)(LPCTSTR)pubKey, (unsigned char*)K2AesEnc, K2AesEncLen, (unsigned char*)K2AesDec, K2AesDecLen);
	
	free(K2AesEnc);

	//TRACE("k2 = %s\n", K2AesDec);

	sPwd = K2AesDec;

	return sPwd;
}

CString CFlashTradeGateway::GetJsonResponse(bool bSuccess, CString type, CString msg)
{
	CString response;
	
	if (bSuccess)
	{
		// 要不要返回sResponse += "{\"cssweb_test\":\"0\"}]}";
		response.Format("{\"cssweb_code\":\"success\",\"cssweb_type\":\"%s\",\"cssweb_msg\":\"%s\",\"item\":[]}", type, msg);
	}
	else
		response.Format("{\"cssweb_code\":\"error\",\"cssweb_type\":\"%s\",\"cssweb_msg\":\"%s\",\"item\":[]}", type, msg);

//	TRACE(response);

	EncryptResponse(true, response);
		
	return response;
}

int CFlashTradeGateway::EncryptResponse(bool bEncrypt, CString& res)
{
	if (bEncrypt == true)
	{
		// 1|base64(aes(k2, response))
		

		int resLen = res.GetLength();

		

		int base64Len = (((resLen+2)/3)*4) + 32; // Base64 text length
		int pemLen = base64Len + base64Len/64; // PEM adds a newline every 64 bytes

		char * base64Res = (char*) malloc(pemLen);
		memset(base64Res, 0x00, pemLen);

		EncodeBase64((unsigned char*)(const char*)res, resLen, (unsigned char*)base64Res, &pemLen);

		
	
		//res = "1|";
		//res += base64Res;

		res = base64Res;


		free(base64Res);
	}

	if (bEncrypt == false)
	{
		//解密出错
		//客户号不相符
		// 2|base64(response)
		
	}

	return 1;

}


bool CFlashTradeGateway::SendRequest(CString sRequest, CString& sResponse)
{
	/*
	GetDecryptRequest(sRequest);

//	g_LogFile.Log("解密后的请求\n");
//	g_LogFile.Log(sRequest);


	CString strNameValue;
	CString strName, strValue;
	int i = 0;
	CString type;
	CString tradePwd;
	CString funcId;

	while (AfxExtractSubString(strNameValue, sRequest, i, '&'))
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

	   // 要注意其它密码的处理
		if (strName == "trdpwd")
		{
			tradePwd = GetTradePWD(strValue);
		}
		if (strName == "cssweb_type")
		{
			type = strValue;
		}
		if (strName == "funcid")
		{
			funcId = strValue;
		}

	}


	bQuote = false;
	int nOldCrypt, nOldCompress, nNewCrypt, nNewCompress;
	int nLen = sizeof(int);

	int nRet = -1;

	KCBPOPHANDLE hReq = NULL;

	KCBPOP_Init( &hReq );

RETRY:
	KCBPOP_BeginWrite(hReq);

	
	i = 0;
	while (AfxExtractSubString(strNameValue, sRequest, i, '&'))
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

		
		TRACE(strName);
		TRACE("=");
		TRACE(strValue);
		TRACE("\n");

			if(strName == "cssweb_type")
			{
				continue;
			}
			else if (strName == "KCBP_PARAM_SERVICENAME")
			{
					KCBPOP_SetSystemParam(hReq, KCBP_PARAM_SERVICENAME, (LPSTR)(LPCTSTR)strValue); // 功能号
					if (strValue == "400400")
					{
						bQuote = true;
					}
					else
					{
						bQuote = false;
					}
			}
			else if (strName == "KCBP_PARAM_RESERVED")
			{
					KCBPOP_SetSystemParam(hReq, KCBP_PARAM_RESERVED, (LPSTR)(LPCTSTR)strValue); // 营业部代码
			}
			else if (strName == "KCBP_PARAM_PACKETTYPE")
			{
					KCBPOP_SetSystemParam(hReq, KCBP_PARAM_PACKETTYPE, (LPSTR)(LPCTSTR)strValue); //包类型
			}
			else if (strName == "trdpwd")
			{
//				CString sEncryptPWD = g_Encrypt.EncryptPWD(strValue);
//						KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)sEncryptPWD);
				CString sEncryptPWD = g_Encrypt.EncryptPWD(tradePwd);
						KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)sEncryptPWD);
			}
			else if (strName == "newpwd") // 410302
			{
				CString sEncryptPWD = g_Encrypt.EncryptPWD(strValue);
						KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)sEncryptPWD);
			}
			else if (strName == "oldfundpwd" || strName=="newfundpwd") // 410303
			{
				CString sEncryptPWD = g_Encrypt.EncryptPWD(strValue);
						KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)sEncryptPWD);
			}
			else if (strName == "fundpwd") // 410606 410607 410605
			{
				CString sEncryptPWD = g_Encrypt.EncryptPWD(strValue);
						KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)sEncryptPWD);
				
			}
			else if (strName == "ram")
			{
				continue;
			}
			else
			{
					KCBPOP_SetValueStr(hReq, (LPSTR)(LPCTSTR)strName, (LPSTR)(LPCTSTR)strValue);
			}
	}

	if (bQuote)
	{
		

		KCBP_GetOption(m_pConn->handle, KCBP_OPTION_CRYPT, &nOldCrypt, &nLen);
	    KCBP_GetOption(m_pConn->handle, KCBP_OPTION_COMPRESS, &nOldCompress, &nLen);

		nNewCrypt = 0;
		nNewCompress = 0;
		
		KCBP_SetOption(m_pConn->handle, KCBP_OPTION_CRYPT, &nNewCrypt, nLen);
		KCBP_SetOption(m_pConn->handle, KCBP_OPTION_COMPRESS, &nNewCompress, nLen);
	}

	KCBPOP_EndWrite(hReq);



	char tmpKCBPPacket[KCBPPACKET_MAXSIZE];
	memset(tmpKCBPPacket, 0x00, sizeof(tmpKCBPPacket));

	int nKCBPPacketLen = -1;

	KCBPOP_GetKCBPPacket(hReq, tmpKCBPPacket, sizeof(tmpKCBPPacket), &nKCBPPacketLen);

	char szMsgID[512];
	memset(szMsgID, 0x00, sizeof(szMsgID));

	KCBPComm_Send(m_pConn->handle, tmpKCBPPacket, nKCBPPacketLen, szMsgID);

	KCBPOP_Exit( hReq );

	sResponse = "";

	if (funcId == "410810")
	{
		if (!RecvResponse2(sResponse, szMsgID, type))
		{
			if (m_nReTry > 0)
			{
				if (m_pConn->CreateConnect(true))
				{
					m_nReTry --;
					goto RETRY;
				}
				else
				{
					sResponse = GetJsonResponse(false, type, "retry failed");
				}
			}
		}
	}
	else
	{
		if (!RecvResponse(sResponse, szMsgID, type))
		{
			// 2054 2055 2003 2004
			
			if (m_nReTry > 0)
			{
				if (m_pConn->CreateConnect(true))
				{
					m_nReTry --;
					goto RETRY;
				}
				else
				{
					sResponse = GetJsonResponse(false, type, "retry failed");
				}
			}
		}
	}



	if (bQuote)
	{
		KCBP_SetOption(m_pConn->handle, KCBP_OPTION_CRYPT, &nOldCrypt, nLen);
		KCBP_SetOption(m_pConn->handle, KCBP_OPTION_COMPRESS, &nOldCompress, nLen);
	}

	if (sResponse.GetLength() <= 0)
	{
		sResponse = GetJsonResponse(true, type, "没有数据");
	}
	else
	{
		// 返回base64(aes(k2, response))
		EncryptResponse(true, sResponse);
	}
*/
	return true;
}

bool CFlashTradeGateway::RecvResponse(CString& sResponse, CString szMsgID, CString type)
{
	/*
	// 接收
	char szErrMsg[2048];
	int nRet = -1;
	

	KCBPOPHANDLE hAns = NULL;
	KCBPOP_Init( &hAns );

	
	while (true)
	{
		int nDataLen = -1;
		char tmpKCBPPacket[KCBPPACKET_MAXSIZE];
		memset(tmpKCBPPacket, 0x00, sizeof(tmpKCBPPacket));

		nRet = KCBPComm_Recv(m_pConn->handle, tmpKCBPPacket, sizeof(tmpKCBPPacket), &nDataLen, (LPSTR)(LPCTSTR)szMsgID);
		if (nRet != 0)
		{
			memset(szErrMsg, 0x00, sizeof(szErrMsg));
			KCBPOP_GetErrorMsg( hAns, szErrMsg, sizeof(szErrMsg)-1 );
			
			CString msg;
			msg.Format("KCBPComm_Recv code=%d msg=%s", nRet, szErrMsg);

			sResponse = GetJsonResponse(false, type, msg);
			return true;
		}

		nRet = KCBPOP_PutKCBPPacket(hAns, tmpKCBPPacket, nDataLen);

		KCBPOP_GetErrorCode( hAns, &nRet);
		if (nRet != 0)
		{
			memset(szErrMsg, 0x00, sizeof(szErrMsg));
			KCBPOP_GetErrorMsg( hAns, szErrMsg, sizeof(szErrMsg)-1 );
			
			CString msg;
			msg.Format("KCBPOP_PutKCBPPacket code=%d msg=%s", nRet, szErrMsg);
			sResponse = GetJsonResponse(false, type, msg);
			return true;
		}


		KCBPOP_RsOpen(hAns);

		nRet = KCBPOP_RsFetchRow(hAns);

		char szCode[1024];
		memset(szCode, 0x00, sizeof(szCode));

		nRet = KCBPOP_RsGetColByName( hAns, "CODE", szCode, sizeof(szCode) );
		
		//return false; test 2054

		if (atoi(szCode) != 0)
		{
			memset(szErrMsg, 0x00, sizeof(szErrMsg));
			//KCBPOP_GetErrorMsg( hAns, szErrMsg, sizeof(szErrMsg)-1 );
			KCBPOP_RsGetColByName( hAns, "MSG", szErrMsg, sizeof(szErrMsg) );

			
			CString msg;
			msg.Format("KCBPOP_RsGetColByName code=%d msg=%s", nRet, szErrMsg);
			sResponse = GetJsonResponse(false, type, msg);

			if (nRet == 2054 || nRet == 2055 || nRet == 2004 || nRet == 2003)
			{
				return false;
			}
			else
				return true;
			
		}

		nRet = KCBPOP_RsMore(hAns);

		int nCol = 0;
		nRet = KCBPOP_RsGetColNum(hAns, &nCol);


	bool hasData = false;

	sResponse = "{";
	sResponse += "\"cssweb_code\":\"success\",";
	sResponse += "\"cssweb_type\":\"" + type + "\",";
	sResponse += "\"item\":[";

		char tmpbuf[512];
		CString name;
		CString value;

		while ( !KCBPOP_RsFetchRow(hAns) )
		{
			hasData = true;
		
			sResponse += "{";


			for( int i = 1; i <= nCol; i++ )
			{
				memset(tmpbuf, 0x00, sizeof(tmpbuf));
				nRet = KCBPOP_RsGetColName( hAns, i, tmpbuf, sizeof(tmpbuf) );
				name = tmpbuf;

				//sResponse.Append(name);
				//sResponse.Append("=");
				

				memset(tmpbuf, 0x00, sizeof(tmpbuf));
				nRet = KCBPOP_RsGetColByIndex( hAns, i, tmpbuf, sizeof(tmpbuf) );
				value = tmpbuf;

				
				//				sResponse.Append(tmpbuf);

				CString sKeyValue = "\"" + name + "\":\"" + value + "\"";

				if (i!=nCol)
				{
					
					sResponse += sKeyValue + ",";
				}
				else
				{
					sResponse += sKeyValue;
				}
			}

			sResponse += "},";
		} // end while

	sResponse += "{\"cssweb_test\":\"0\"}]}";
	
	if (!hasData)
	{
		sResponse.Empty();
	}
		

		if (KCBPOP_EndOfTran(hAns))
			break;
	}

	
	nRet = KCBPOP_Exit( hAns );
	*/
	return true;
}

