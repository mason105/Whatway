#ifndef MSG_HEADER_H
#define MSG_HEADER_H
/*
直接采用二进制协议
行情和交易统一
将来和IM统一
交易tcp和ssl统一
全部基于改为基于会话模式通信
要支持加密模式吗?
*/

//如果要支持除ssl外的会话加密钥协议，每个socket分配一个aes密钥

// 推送如何处理？

#define MSG_TYPE_REQUEST 0 // 请求
#define MSG_TYPE_RESPONSE_ERROR 1 // 应答错误
#define MSG_TYPE_RESPONSE_MORE 2 // 应答继续
#define MSG_TYPE_RESPONSE_END 3 // 应答结束
#define MSG_TYPE_RESPONSE_PUSH 4 // 服务器推送

#define FUNCTION_HEARTBEAT 0 // 心跳
#define FUNCTION_CAPTCHA 1 // 验证码


/*
行情 请求/应答
     请求/推送

交易 请求/应答

*/

// 1字节对齐
#pragma pack(1)
typedef struct MsgHeader
{
	int MsgContentSize; // 消息内容大小
	int CRC; // 保留
	unsigned char zip; // 是否支持压缩
	unsigned char MsgType; //消息类型 
	int FunctionNo; // 功能号

	//unsigned char SystemId; // 客户端系统编号
	//unsigned char SystemVersion; // 客户端终端类型
	//unsigned char BusinessType; // 业务类型
}MSG_HEADER, *PMSG_HEADER;

#pragma pack()

// 错误应答
/*
int nErrCode;
char szErrMsg; [pkgbodysize - sizeof(int)]
*/

#endif
