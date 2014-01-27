#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "./log/tradelog.pb.h"
#include "./connectpool/Connect.h"
#include <map>


//#include "trade.pb.h"


#include "./lib/hundsun_t2/t2sdk_interface.h"
#include "./connectpool/Connect.h"
#include "./business/IBusiness.h"


class TradeBusinessT2 : public IBusiness
{
public:
	TradeBusinessT2();
	~TradeBusinessT2(void);

	virtual bool CreateConnect();
	virtual bool Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg);


	char * aeskey;
	std::string m_sNewPwd;
	std::string GetTradePWD(std::string isSafe, std::string sEncPwd);
	std::string GetOtherPWD(std::string isSafe, std::string sEncPwd);
	std::string trdpwd;
	std::string newpwd;
	std::string newpwd_enc;

	

	// ∫„…˙T2
	CConfigInterface * lpConfig;
	CConnectionInterface * lpConnection;
	
};
