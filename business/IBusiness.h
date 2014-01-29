#ifndef _IBUSINESS_H_
#define _IBUSINESS_H_

// stl
#include <string>
#include <map>
#include <vector>
// boost
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "common.h"
#include "./log/tradelog.pb.h"
#include "./connectpool/Connect.h"
#include "ConnectPool/Counter.h"

class IBusiness
{
public:
	IBusiness(void);
	~IBusiness(void);

	bool m_bConnected;
	virtual bool IsConnected();

	Counter * m_Counter;
	virtual void SetCounterServer(Counter * counter);

	virtual bool CreateConnect() = 0;
	virtual void CloseConnect() = 0;

	virtual bool Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg) = 0;


	
	
	

	


	std::string sysNo;

	int busiType;
	std::string bt;

	std::string sysVer;

	std::string branchId;

	std::string funcid;
	long lFuncId;

	std::string route;
	int nRoute;

	std::string SOH;

	std::string account; // 客户号，用于命名日志文件

	std::string captcha;

	std::string note;

	std::string hardinfo;

	std::string cssweb_cacheFlag;

	std::string cssweb_flashCallback;
	std::string cssweb_pwdType;

	std::string cssweb_connid;

	std::map<std::string, std::string> reqmap;

	

	std::string sCounterType;

	// flash交易，修改密码功能，普通模式，产生的新密码需要返回给前端
	std::string flash_normal_modifypwd_newpwd;


	// 日志共用变量定义
	boost::posix_time::ptime beginTime; // 开始时间
	std::string sBeginTime;
	//boost::posix_time::ptime endTime; // 开始时间
	Trade::TradeLog::LogLevel logLevel; // 日志级别
	std::string logEncodeRequest; // 请求密文, flash交易
	std::string logRequest; // 请求明文
	int64_t runtime; // 运行时间
	int status;
	std::string retcode;
	std::string retmsg;
	std::string logResponse; // 响应明文
	std::string logEncodeResponse; // 响应密文, flash交易
	std::string gt_ip; // 柜台ip
	std::string gt_port; // 柜台port
	std::string gateway_ip; // 网关ip
	std::string gateway_port; // 网关port

	void ParseRequest(std::string& request);
	bool FilterRequestField(std::string& key);
	
	void RetErrRes(Trade::TradeLog::LogLevel logLevel, std::string& response, std::string retcode, std::string retmsg);
	void RetNoRecordRes(std::string& response);

	

	int ConvertIntToBusiType(int val);
	
	
	

	bool DecryptPassword(std::string algo, std::string key, std::string cipher, std::string plain);

	
	void GenResponse(int nErrCode, std::string sErrMsg, std::string& response, int& status, std::string& errCode, std::string& errMsg);
};

#endif
