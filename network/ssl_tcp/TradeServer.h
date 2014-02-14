#ifndef _TRADE_SERVER_TCP_
#define _TRADE_SERVER_TCP_

#include "network/ssl_tcp/CustomMessage.h"
#include "ThreadSafeQueue/job_queue.h"
#include "./threadpool/worker.h"

//#include "network/TcpSession.h"

#include "common.h"
#include "ocr/ocr.h"


class TradeServer
{
public:
	typedef job_queue<IMessage*> req_queue_type;
	typedef worker<req_queue_type> req_worker_type;

	typedef job_queue<IMessage*> resp_queue_type;
	typedef worker<resp_queue_type> resp_worker_type;

private:
	req_queue_type recvq_;
	resp_queue_type sendq_;

	req_worker_type req_worker_;
	resp_worker_type resp_worker_;


public:
	TradeServer(int msgType);
	req_queue_type& recv_queue();
	void start();
	void stop();

private:
	// 处理请求消息
	bool ProcessRequest(IMessage* req);

	// 处理应答消息
	bool ProcessResponse(IMessage* resp);


	bool GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType, std::string& sysVer, std::string& account, std::string& funcId, std::string& clientIp);


	std::string GetCounterType(int counterType);
	int m_MsgType;

	OCR ocr;


};
#endif
