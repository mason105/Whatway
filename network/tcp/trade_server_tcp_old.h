#ifndef _TRADE_SERVER_TCP_OLD_
#define _TRADE_SERVER_TCP_OLD_

#include "tcp_message_old.h"
#include "ThreadSafeQueue/job_queue.h"
#include "./threadpool/worker.h"
#include "tcp_session_old.h"




class trade_server_tcp_old
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
	trade_server_tcp_old();
	req_queue_type& recv_queue();
	void start();
	void stop();

private:
	bool process_msg(IMessage* req);

	bool send_msg(IMessage* resp);


	bool GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType, bool& bIsHeartBeat);



};
#endif
