/** @file
* 演示T2_SDK进行打包、发包、收包、解包
* @author  T2小组
* @author  恒生电子股份有限公司
* @version 1.0
* @date    20090327
*/
#include <Include/t2sdk_interface.h>

CConnectionInterface *g_lpConnection = NULL;
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
int main()
{
	
	//通过T2SDK的引出函数，来获取一个新的CConfig对象指针
	//此对象在创建连接对象时被传递，用于配置所创建的连接对象的各种属性（比如服务器IP地址、安全模式等）
	//值得注意的是，在向配置对象设置配置信息时，配置信息既可以从ini文件中载入，
	//也可以在程序代码中设定，或者是2者的混合，如果对同一个配置项设不同的值，则以最近一次设置为准
	CConfigInterface * lpConfig = NewConfig();
	lpConfig->AddRef();
	lpConfig->Load("publisher.ini");
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
			char szFilterName[6][100]={0};
			char szFilterValue[6][100]={0};
			char* topicName = (char*)lpConfig->GetString("publish","topic_name","");//获取发布的主题名


			//获取过滤字段名字和值
			int nCount = lpConfig->GetInt("publish","filter_count",0);
			for (int i=1;i<=nCount;i++)
			{
				char lName[128]={0};
				sprintf(lName,"filter_name%d",i);
				char* filterName = (char*)lpConfig->GetString("publish",lName,"");
				char lValue[128]={0};
				sprintf(lValue,"filter_value%d",i);
				char* filterValue = (char*)lpConfig->GetString("publish",lValue,"");

				strncpy(szFilterName[i-1],filterName,99);
				strncpy(szFilterValue[i-1],filterValue,99);
			}

			printf("构造发布消息\n");
			//如果需要使用发布订阅功能，必须配置配置mc标签下面的client_name项
			CPublishInterface* lpPublish = g_lpConnection->GetPublisher(200,5000);
			if (!lpPublish)
			{
				printf("NewPublish Error: %s\n",g_lpConnection->GetMCLastError());
				return -1;
			}
			printf("开始发布\n");


			//构造发布的业务包内容
			IF2Packer* lpOnePack = NewPacker(2);
			lpOnePack->AddRef();
			lpOnePack->BeginPack();	
			for (int j=0;j<nCount;j++)
			{
				lpOnePack->AddField(szFilterName[j]);
			}

			for (int k=0;k<nCount;k++)
			{
				lpOnePack->AddStr(szFilterValue[k]);
			}

			lpOnePack->EndPack();

			IF2UnPacker* lpUnPack = lpOnePack->UnPack();

			while (1)
			{
				//业务包构造完毕
				//调用业务的发送接口进行发布
				int iRet = lpPublish->PubMsgByPacker(topicName,lpUnPack,5000,NULL/*,true,&uRecordTime*/);
				//打印错误信息
				printf("Next %d,%s,MsgID:%d\n",iRet,g_lpConnection->GetErrorMsg(iRet),lpPublish->GetMsgNoByTopicName(topicName));

				printf("是否退出，[y]退出，[其他字符]继续\n"); 
				char ch=getchar();
				if (ch=='y')
				{
					break;
				}
			}

			lpOnePack->FreeMem(lpOnePack->GetPackBuf());
			lpOnePack->Release();

			//释放发布端
			lpPublish->Release();
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
