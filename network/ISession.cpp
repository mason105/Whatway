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
}


ISession::~ISession(void)
{
}

void ISession::CreateCounterConnect(Counter * counter)
{
		switch(counter->m_eCounterType)
		{
		case COUNTER_TYPE_HS_T2:
			counterT2 = new TradeBusinessT2();
			break;
		case COUNTER_TYPE_HS_COM:
			break;
		case COUNTER_TYPE_JZ_WIN:
			counterSzkingdom = new TradeBusiness();
			break;
		case COUNTER_TYPE_JZ_LINUX:
			break;
		case COUNTER_TYPE_DINGDIAN:
			counterApex = new TradeBusinessDingDian();
			break;
		case COUNTER_TYPE_JSD:
			{
			counterAGC = new CSywgConnect();
			
			break;
			}
		case COUNTER_TYPE_XINYI:
			{
			counterXinYi = new CTCPClientSync();
			
			break;
			}
		default:
			break;
		}
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

IBusiness * ISession::GetCounterConnect(int counterType)
{
	IBusiness * business = NULL;

	switch(counterType)
	{
	case 0:
		business = counterT2;
		break;

	}

	return business;
}
