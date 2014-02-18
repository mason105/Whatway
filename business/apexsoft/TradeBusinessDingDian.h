#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "./log/tradelog.pb.h"

#include <map>


//#include "trade.pb.h"




#include "./business/IBusiness.h"


class TradeBusinessDingDian : public IBusiness
{
public:
	TradeBusinessDingDian();
	~TradeBusinessDingDian(void);

	virtual bool CreateConnect();
	virtual void CloseConnect();
	virtual bool Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg);


private:
	char* EncryptPwd(const char* plainText, char* szPwd);
	long m_hHandle;
};
