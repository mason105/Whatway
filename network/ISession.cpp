#include "stdafx.h"

#include "ISession.h"


ISession::ISession(void)
{
	counterT2 = NULL;
	counterSzkingdom = NULL;
	counterApex = NULL;
	counterAGC = NULL;
	counterXinYi = NULL;
}


ISession::~ISession(void)
{
}

void ISession::CloseCounterConnect()
{
	if (counterT2 != NULL)
	{
		counterT2->CloseConnect();
		delete counterT2;
		counterT2 = NULL;
	}

	if (counterSzkingdom != NULL)
	{
		counterSzkingdom->CloseConnect();
		delete counterSzkingdom;
		counterSzkingdom = NULL;
	}

	if (counterApex != NULL)
	{
		counterApex->CloseConnect();
		delete counterApex;
		counterApex = NULL;
	}

	if (counterAGC != NULL)
	{
		counterAGC->CloseConnect();
		delete counterAGC;
		counterAGC = NULL;
	}

	if (counterXinYi != NULL)
	{
		counterXinYi->CloseConnect();
		delete counterXinYi;
		counterXinYi = NULL;
	}
}

IBusiness * ISession::GetCounterConnect()
{
	return NULL;
}

void ISession::CreateCounterConnect()
{
}
