#include "stdafx.h"

#include "ISession.h"


// 金证
#include "business/szkingdom/tradebusiness.h"
#include "business/hundsun_com/TradeBusinessComm.h"
// 恒生
#include "business/hundsun_t2/tradebusinesst2.h"
// 顶点
#include "business/apexsoft/TradeBusinessDingDian.h"
#include "business/apexsoft/DingDian.h"
// AGC
#include "business/SunGuard/SywgConnect.h"
// 新意
#include "business/xinyi/TCPClientSync.h"


ISession::ISession(void)
{
	counterT2 = NULL;
	counterSzkingdom = NULL;
	counterApex = NULL;
	counterAGC = NULL;
	counterXinYi = NULL;

	counterT2 = new TradeBusinessT2();
	counterSzkingdom = new TradeBusiness();
	counterApex = new TradeBusinessDingDian();
	counterAGC = new CSywgConnect();
	counterXinYi = new CTCPClientSync();
}


ISession::~ISession(void)
{
	if (counterT2 != NULL)
	{
		delete counterT2;
		counterT2 = NULL;
	}

	if (counterSzkingdom != NULL)
	{
		delete counterSzkingdom;
		counterSzkingdom = NULL;
	}

	if (counterApex != NULL)
	{
		delete counterApex;
		counterApex = NULL;
	}

	if (counterAGC != NULL)
	{
		delete counterAGC;
		counterAGC = NULL;
	}

	if (counterXinYi != NULL)
	{
		delete counterXinYi;
		counterXinYi = NULL;
	}
}


void ISession::CloseCounterConnect()
{
	if (counterT2 != NULL)
	{
		counterT2->CloseConnect();
		
	}

	if (counterSzkingdom != NULL)
	{
		counterSzkingdom->CloseConnect();
		
	}

	if (counterApex != NULL)
	{
		counterApex->CloseConnect();
		
	}

	if (counterAGC != NULL)
	{
		counterAGC->CloseConnect();
		
	}

	if (counterXinYi != NULL)
	{
		counterXinYi->CloseConnect();
		
	}
}

// 根据参数，返回对应的柜台连接
IBusiness * ISession::GetCounterConnect(int counterType)
{

	IBusiness * business = NULL;

	switch(counterType)
	{
	case COUNTER_TYPE_HS_T2:
		business = counterT2;
		break;
	case COUNTER_TYPE_HS_COM:
		
		break;
	case COUNTER_TYPE_JZ_WIN:
		business = counterSzkingdom;
		break;
	case COUNTER_TYPE_JZ_LINUX:
		
		break;
	case COUNTER_TYPE_DINGDIAN:
		business = counterApex;
		break;
	case COUNTER_TYPE_JSD:
		business = counterAGC;
		break;
	case COUNTER_TYPE_XINYI:
		business = counterXinYi;
		break;
	
	}

	return business;
}
