#include "http_message.h"
#include "ThreadSafeQueue/job_queue.h"
#include "./threadpool/worker.h"
#include "http_session.h"


#include <boost/functional/factory.hpp>


class trade_server_http
{
public:
	typedef job_queue<http_request_ptr> req_queue_type;
	typedef worker<req_queue_type> req_worker_type;

	typedef job_queue<http_response_ptr> resp_queue_type;
	typedef worker<resp_queue_type> resp_worker_type;

private:
	req_queue_type recvq_;
	resp_queue_type sendq_;

	req_worker_type req_worker_;
	resp_worker_type resp_worker_;


public:
	trade_server_http();
	req_queue_type& recv_queue();
	void start();
	void stop();
	void log();

private:
	bool process_msg(http_request_ptr req);
	void invalid_request(http_request_ptr req);
	void crossdomain(http_request_ptr req);

	bool send_msg(http_response_ptr resp);


	
	Trade::TradeLog logMsg;

	bool GetSysNoAndBusiType(std::string& request, std::string& sysNo, std::string& busiType);

};
