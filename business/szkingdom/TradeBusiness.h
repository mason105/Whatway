#include <string>
#include <map>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "./log/tradelog.pb.h"
#include "./connectpool/Connect.h"



//#include "trade.pb.h"

#include "./connectpool/Connect.h"
#include "./business/IBusiness.h"


class TradeBusiness : public IBusiness
{
public:
	TradeBusiness();
	~TradeBusiness(void);

	void Process(std::string& request, std::string& response, Trade::TradeLog& logMsg);
	


	char * aeskey;
	std::string m_sNewPwd;
	std::string GetTradePWD(std::string isSafe, std::string sEncPwd);
	std::string GetOtherPWD(std::string isSafe, std::string sEncPwd);
	std::string trdpwd;
	std::string newpwd;
	std::string newpwd_enc;
};
