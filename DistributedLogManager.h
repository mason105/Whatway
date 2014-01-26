#ifndef DISTRIBUTED_LOG_MANAGER_H
#define DISTRIBUTED_LOG_MANAGER_H

#include "ThreadSafeQueue/job_queue.h"
#include "threadpool/worker.h"
#include "ThreadSafeQueue/job_queue.h"
#include "threadpool/worker.h"
#include "log/tradelog.pb.h"

typedef struct func
{
	bool isQuery;
	bool hasResultRet;
}FUNCTION_DESC;


class DistributedLogManager
{
public:
	DistributedLogManager(void);
	~DistributedLogManager(void);

	typedef job_queue<Trade::TradeLog*> kafka_queue_type;
	typedef worker<kafka_queue_type> kafka_worker_type;

	kafka_queue_type kafka_q_;
	kafka_worker_type kafka_worker_;

	void start();
	void stop();
	void push(Trade::TradeLog * log);

	
	bool kafka_log(Trade::TradeLog* log);

	
	

	// 过滤字段
	std::map<std::string, std::string> m_mDingDian_FilterField;
	std::map<std::string, std::string> m_mT2_FilterField;
	std::map<std::string, std::string> m_mKingdom_FilterField;
	void LoadFieldFilter();
	void LoadFieldFilter(std::string& sFieldFilterXML, std::map<std::string, std::string>& mapFieldFilter);
	void GetFilterMap(std::string& request, std::map<std::string, std::string>& mapFieldFilter, std::map<std::string, std::string>& reqmap);

	// 过滤功能号
	std::map<std::string, FUNCTION_DESC> m_mDingDian_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mT2_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mKingdom_FilterFunc;
	void LoadFuncFilter();
	void LoadFuncFilter(std::string& sFuncFilterXML, std::map<std::string, FUNCTION_DESC>& mapFuncFilter);
};

#endif
