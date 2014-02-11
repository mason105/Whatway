#pragma once

#include "Connect.h"

class CFlashTradeGateway
{
public:
	CFlashTradeGateway(CConnect* pConn);
	~CFlashTradeGateway();

private:
	//CString m_type;

	CConnect* m_pConn;
	int m_nReTry;
	

public:
	bool SendRequest(CString sRequest, CString& sResponse);

	CString GetJsonResponse(bool bSuccess, CString type, CString msg);


	bool GetDecryptRequest(CString& request);

	CString GetTradePWD(CString sEncPwd);
	int EncryptResponse(bool bEncrypt, CString& sResponse);

	CString k2base64;
	bool RecvResponse(CString& sResponse, CString sMsgId, CString type);
	

};

