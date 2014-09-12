/** @file
* 演示T2_SDK进行打包、发包、收包、解包
* @author  T2小组
* @author  恒生电子股份有限公司
* @version 1.0
* @date    20090327
*/
#include <Include/t2sdk_interface.h>
#include <map>
using namespace std;

map<int, CSubscribeParamInterface*> g_allSubscribeParam;
CConnectionInterface *g_lpConnection = NULL;

void PrintUnPack(IF2UnPacker* lpUnPack)
{
	printf("记录行数：           %d\n",lpUnPack->GetRowCount());
	printf("列行数：			 %d\n",lpUnPack->GetColCount());
	while (!lpUnPack->IsEOF())
	{
		for (int i=0;i<lpUnPack->GetColCount();i++)
		{
			char* colName = (char*)lpUnPack->GetColName(i);
			char colType = lpUnPack->GetColType(i);
			if (colType!='R')
			{
				char* colValue = (char*)lpUnPack->GetStrByIndex(i);
				printf("%s:			[%s]\n",colName,colValue);
			}
			else
			{
				int colLength = 0;
				char* colValue = (char*)lpUnPack->GetRawByIndex(i,&colLength);
				printf("%s:			[%s](%d)\n",colName,colValue,colLength);
			}
		}
		lpUnPack->Next();
	}

}

void PrintSub(int subIndex,LPSUBSCRIBE_RECVDATA lpRecvData)
{
	map<int, CSubscribeParamInterface*>::iterator itr = g_allSubscribeParam.find(subIndex);
	if(itr==g_allSubscribeParam.end())
	{
		printf("没有这个订阅项\n");
		return ;
	}
	CSubscribeParamInterface* lpSubParam = (*itr).second;

	printf("----------订阅项部分-------\n");
	printf("主题名字：           %s\n",lpSubParam->GetTopicName());
	printf("附加数据长度：       %d\n",lpRecvData->iAppDataLen);
	if (lpRecvData->iAppDataLen>0)
	{
		printf("附加数据：           %s\n",lpRecvData->lpAppData);
	}
	printf("过滤字段部分：\n");
	if(lpRecvData->iFilterDataLen>0)
	{
		IF2UnPacker* lpUnpack = NewUnPacker(lpRecvData->lpFilterData,lpRecvData->iFilterDataLen);
		lpUnpack->AddRef();
		PrintUnPack(lpUnpack);
		lpUnpack->Release();
	}

	printf("---------------------------\n");	

}


class CCallback : public CCallbackInterface
{
public:
	 unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv);
	 unsigned long  FUNCTION_CALL_MODE AddRef();
	 unsigned long  FUNCTION_CALL_MODE Release();
     void FUNCTION_CALL_MODE OnConnect(CConnectionInterface *lpConnection);
     void FUNCTION_CALL_MODE OnSafeConnect(CConnectionInterface *lpConnection);
     void FUNCTION_CALL_MODE OnRegister(CConnectionInterface *lpConnection);
     void FUNCTION_CALL_MODE OnClose(CConnectionInterface *lpConnection);
     void FUNCTION_CALL_MODE OnSent(CConnectionInterface *lpConnection, int hSend, void *reserved1, void *reserved2, int nQueuingData);
     void FUNCTION_CALL_MODE Reserved1(void *a, void *b, void *c, void *d);
     void FUNCTION_CALL_MODE Reserved2(void *a, void *b, void *c, void *d);
     int  FUNCTION_CALL_MODE Reserved3();
     void FUNCTION_CALL_MODE Reserved4();
     void FUNCTION_CALL_MODE Reserved5();
     void FUNCTION_CALL_MODE Reserved6();
     void FUNCTION_CALL_MODE Reserved7();
     void FUNCTION_CALL_MODE OnReceivedBiz(CConnectionInterface *lpConnection, int hSend, const void *lpUnPackerOrStr, int nResult);
     void FUNCTION_CALL_MODE OnReceivedBizEx(CConnectionInterface *lpConnection, int hSend, LPRET_DATA lpRetData, const void *lpUnpackerOrStr, int nResult);
	 void FUNCTION_CALL_MODE OnReceivedBizMsg(CConnectionInterface *lpConnection, int hSend, IBizMessage* lpMsg);

};
//以下各回调方法的实现仅仅为演示使用
unsigned long CCallback::QueryInterface(const char *iid, IKnown **ppv)
{
	return 0;
}

unsigned long CCallback::AddRef()
{
	return 0;
}

unsigned long CCallback::Release()
{
	return 0;
}
void CCallback::OnConnect(CConnectionInterface *lpConnection)
{
	puts("CCallback::OnConnect");
}

void CCallback::OnSafeConnect(CConnectionInterface *lpConnection)
{
	puts("CCallback::OnSafeConnect");
}

void CCallback::OnRegister(CConnectionInterface *lpConnection)
{
	puts("CCallback::OnRegister");
}

void CCallback::OnClose(CConnectionInterface *lpConnection)
{
	puts("CCallback::OnClose");
}

void CCallback::OnSent(CConnectionInterface *lpConnection, int hSend, void *reserved1, void *reserved2, int nQueuingData)
{
	
}

void CCallback::OnReceivedBiz(CConnectionInterface *lpConnection, int hSend, const void *lpUnpackerOrStr, int nResult)
{
	
}
void CCallback::OnReceivedBizEx(CConnectionInterface *lpConnection, int hSend, LPRET_DATA lpRetData, const void *lpUnpackerOrStr, int nResult)
{

}
void CCallback::OnReceivedBizMsg(CConnectionInterface *lpConnection, int hSend, IBizMessage* lpMsg)
{

}
void CCallback::Reserved1(void *a, void *b, void *c, void *d)
{
}

void CCallback::Reserved2(void *a, void *b, void *c, void *d)
{
}
int  CCallback::Reserved3()
{
	return 0;
}

void CCallback::Reserved4()
{
}

void CCallback::Reserved5()
{
}

void CCallback::Reserved6()
{
}

void CCallback::Reserved7()
{
}
class CSubCallback : public CSubCallbackInterface
{
	unsigned long  FUNCTION_CALL_MODE QueryInterface(const char *iid, IKnown **ppv)
	{
		return 0;
	}
	unsigned long  FUNCTION_CALL_MODE AddRef()
	{
		return 0;
	}
	unsigned long  FUNCTION_CALL_MODE Release()
	{
		return 0;
	}

	void FUNCTION_CALL_MODE OnReceived(CSubscribeInterface *lpSub,int subscribeIndex, const void *lpData, int nLength,LPSUBSCRIBE_RECVDATA lpRecvData);
	void FUNCTION_CALL_MODE OnRecvTickMsg(CSubscribeInterface *lpSub,int subscribeIndex,const char* TickMsgInfo);
};





void CSubCallback::OnReceived(CSubscribeInterface *lpSub,int subscribeIndex, const void *lpData, int nLength,
	LPSUBSCRIBE_RECVDATA lpRecvData)
{
	printf("***************************\n");
	PrintSub(subscribeIndex,lpRecvData);
	IF2UnPacker* lpUnPack = NewUnPacker((void*)lpData,nLength);
	if (lpUnPack)
	{
		lpUnPack->AddRef();
		PrintUnPack(lpUnPack);
		lpUnPack->Release();
	}
	printf("***************************\n");
}
void CSubCallback::OnRecvTickMsg(CSubscribeInterface *lpSub,int subscribeIndex,const char* TickMsgInfo)
{

}


int main()
{
	//通过T2SDK的引出函数，来获取一个新的CConfig对象指针
	//此对象在创建连接对象时被传递，用于配置所创建的连接对象的各种属性（比如服务器IP地址、安全模式等）
	//值得注意的是，在向配置对象设置配置信息时，配置信息既可以从ini文件中载入，
	//也可以在程序代码中设定，或者是2者的混合，如果对同一个配置项设不同的值，则以最近一次设置为准
	CConfigInterface * lpConfig = NewConfig();
	lpConfig->AddRef();
	lpConfig->Load("subscriber.ini");
	//如果需要使用发布订阅功能，必须配置配置mc标签下面的client_name项，配置文件里面有了，不需要下面这句代码添加
	//lpConfig->SetString("mc","client_name","xuxp");

	//通过T2SDK的引出函数，来获取一个新的CConnection对象指针
	g_lpConnection = NewConnection(lpConfig);
	g_lpConnection->AddRef();

	//创建自定义类CCallback的对象（在创建连接时需传递此对象，请看下面代码）
	CCallback callback;

	int ret = 0;

	//初始化连接对象，返回0表示初始化成功，注意此时并没开始连接服务器
	if (0 == (ret = g_lpConnection->Create2BizMsg(&callback)))
	{
		//正式开始连接，参数1000为超时参数，单位是ms
		if (ret = g_lpConnection->Connect(1000))
		{
			puts(g_lpConnection->GetErrorMsg(ret));
		}
		else
		{
			CSubCallback subscriberCallback;
			char* bizName = (char*)lpConfig->GetString("subcribe","biz_name","");
			//如果需要使用发布订阅功能，必须配置配置mc标签下面的client_name项
			CSubscribeInterface* lpSub = g_lpConnection->NewSubscriber(&subscriberCallback,bizName,5000);
			if (!lpSub)
			{
				printf("NewSubscribe Error: %s\n",g_lpConnection->GetMCLastError());
				return -1;
			}
			lpSub->AddRef();


			//订阅参数获取
			CSubscribeParamInterface* lpSubscribeParam = NewSubscribeParam();
			lpSubscribeParam->AddRef();
			char* topicName = (char*)lpConfig->GetString("subcribe","topic_name","");//主题名字
			lpSubscribeParam->SetTopicName(topicName);
			char* isFromNow = (char*)lpConfig->GetString("subcribe","is_rebulid","");//是否补缺
			if (strcmp(isFromNow,"true")==0)
			{
				lpSubscribeParam->SetFromNow(true);
			}
			else
			{
				lpSubscribeParam->SetFromNow(false);
			}

			char* isReplace = (char*)lpConfig->GetString("subcribe","is_replace","");//是否覆盖
			if (strcmp(isReplace,"true")==0)
			{
				lpSubscribeParam->SetReplace(true);
			}
			else
			{
				lpSubscribeParam->SetReplace(false);
			}

			char* lpApp = "xuxinpeng";
			lpSubscribeParam->SetAppData(lpApp,9);//添加附加数据

			//添加过滤字段
			int nCount = lpConfig->GetInt("subcribe","filter_count",0);
			for (int i=1;i<=nCount;i++)
			{
				char lName[128]={0};
				sprintf(lName,"filter_name%d",i);
				char* filterName = (char*)lpConfig->GetString("subcribe",lName,"");
				char lValue[128]={0};
				sprintf(lValue,"filter_value%d",i);
				char* filterValue = (char*)lpConfig->GetString("subcribe",lValue,"");
				lpSubscribeParam->SetFilter(filterName,filterValue);
			}
			//添加发送频率
			lpSubscribeParam->SetSendInterval(lpConfig->GetInt("subcribe","send_interval",0));
			//添加返回字段
			nCount = lpConfig->GetInt("subcribe","return_count",0);
			for (int k=1;k<=nCount;k++)
			{
				char lName[128]={0};
				sprintf(lName,"return_filed%d",k);
				char* filedName = (char*)lpConfig->GetString("subcribe",lName,"");
				lpSubscribeParam->SetReturnFiled(filedName);
			}


			int subscribeIndex = 0;
			printf("开始订阅\n");
			int  iRet = lpSub->SubscribeTopic(lpSubscribeParam,5000,NULL);
			if(iRet>0)
			{
				subscribeIndex = iRet;
				printf("SubscribeTopic info:[%d] 成功\n",iRet);
				g_allSubscribeParam[subscribeIndex] = lpSubscribeParam;//保存到map中，用于以后的取消订阅
			}
			else
			{
				printf("SubscribeTopic info:[%d] %s\n",iRet,g_lpConnection->GetErrorMsg(iRet));
				return-1;
			}


			//打印已经订阅的主题信息
			printf("**********************************************\n");
			IF2Packer* lpPack = NewPacker(2);
			lpPack->AddRef();
			lpSub->GetSubcribeTopic(lpPack);
			if (lpPack)
			{
				PrintUnPack(lpPack->UnPack());
			}
			lpPack->FreeMem(lpPack->GetPackBuf());
			lpPack->Release();
			printf("**********************************************\n");
			printf("输入任意字符取消订阅\n");
			getchar();


			//添加过滤字段
			iRet = lpSub->CancelSubscribeTopic(subscribeIndex);
			printf("CancelSubscribeTopic:%d %s\n",iRet,g_lpConnection->GetErrorMsg(iRet));
			printf("输入任意字符退出\n");
			getchar();

			//释放订阅端
			lpSub->Release();
			printf("退出\n");
		}	
	}
	else
	{
		puts(g_lpConnection->GetErrorMsg(ret));
	}

	//通过getchar阻塞线程，等待服务端应答包到达
	getchar();

	g_lpConnection->Release();
	lpConfig->Release();
	return 0;
}
