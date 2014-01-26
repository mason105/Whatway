#pragma once

#include "Connect.h"

class CWebTradeGateway
{
public:
	CWebTradeGateway(CConnect* pConn);
	~CWebTradeGateway();
	void SendRequest(CString sRequest, CString& sResponse);

private:
	CConnect* m_pConn;
	int m_nReTry;
};

