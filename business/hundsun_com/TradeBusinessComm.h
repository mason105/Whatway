#include <string>
#include <map>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "./log/tradelog.pb.h"
#include "./connectpool/Connect.h"






#include "business/hundsun_t2/t2sdk_interface.h"
#include "./connectpool/Connect.h"
#include "./business/IBusiness.h"


class TradeBusinessComm /*: public IBusiness*/
{
public:
	TradeBusinessComm(std::string type);
	~TradeBusinessComm(void);

	std::string type; // trade type
	std::string flashreqcallback;
	std::string isSafe;
	char * aeskey;
	std::string m_sNewPwd;
	std::string m_sIphoneFlag;

	std::string funcid;
	std::string branchNo;
	int systemno;

	
	std::map<std::string, std::string> reqmap;

	int row; // 返回行数

	
	long session; 

	void BuildResponseSuccess(std::string& response);

	void Process(std::string& request, std::string& response, Trade::TradeLog& logMsg);
	void RetErrRes(Trade::TradeLog::LogLevel logLevel, std::string& response, std::string retcode, std::string retmsg);

	void Log(Trade::TradeLog& logMsg);
	

	bool m_bEnableSendLog;

	std::string GetTradePWD(std::string isSafe, std::string sEncPwd);
	std::string GetOtherPWD(std::string isSafe, std::string sEncPwd);

	

//	std::string& request;
//	std::string& response;
//	trade::TradeResponse& response_pb;

	std::string trdpwd;
	std::string newpwd;
	std::string newpwd_enc;

	std::string GetType();



	// 日志处理


	boost::posix_time::ptime beginTime;
	
	std::string source;
	
	std::string clientIp;

	Trade::TradeLog::LogLevel logLevel;

	std::string logEncodeRequest; //

	// 交易日志信息

	std::string logRequest; //base64decode

	

	int64_t runtime;
	
	int status;

	std::string retcode;

	std::string retmsg;

	std::string logResponse;

	std::string logEncodeResponse; // base64encode(response)

	std::string account;

	std::string gt_ip;
	std::string gt_port;
	std::string gateway_ip;
	std::string gateway_port;

	// end 日志处理

	char SOH;
	char LF;

	
	
	

	void ComputePackID(long* lPackID);
};
