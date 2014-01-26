#ifndef FILE_LOG_MANAGER_H
#define FILE_LOG_MANAGER_H

#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/detail/singleton.hpp>


#include "ThreadSafeQueue/job_queue.h"
#include "threadpool/worker.h"
#include "log/tradelog.pb.h"

typedef struct func
{
	bool isQuery;
	bool hasResultRet;
}FUNCTION_DESC;

/*
开发计划
要以共享方式打开文件
*/

class FileLogManager
{
public:
	FileLogManager(void);
	~FileLogManager(void);

	typedef job_queue<Trade::TradeLog> file_queue_type;
	typedef worker<file_queue_type> file_worker_type;

	file_queue_type file_q_;
	file_worker_type file_worker_;

	bool file_log(Trade::TradeLog log);

	void push(Trade::TradeLog log);

	void start();
	void stop();

	// 过滤字段
	std::map<std::string, std::string> m_mDingDian_FilterField;
	std::map<std::string, std::string> m_mT2_FilterField;
	std::map<std::string, std::string> m_mKingdom_FilterField;
	std::map<std::string, std::string> m_mAGC_FilterField;
	std::map<std::string, std::string> m_mXinyi_FilterField;
	void LoadFieldFilter();
	void LoadFieldFilter(std::string& sFieldFilterXML, std::map<std::string, std::string>& mapFieldFilter);
	void GetFilterMap(std::string& request, std::map<std::string, std::string>& mapFieldFilter, std::map<std::string, std::string>& reqmap);

	// 过滤功能号
	std::map<std::string, FUNCTION_DESC> m_mDingDian_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mT2_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mKingdom_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mAGC_FilterFunc;
	std::map<std::string, FUNCTION_DESC> m_mXinyi_FilterFunc;
	void LoadFuncFilter();
	void LoadFuncFilter(std::string& sFuncFilterXML, std::map<std::string, FUNCTION_DESC>& mapFuncFilter);
	
};
typedef boost::detail::thread::singleton<FileLogManager> gFileLogManager;

#endif
