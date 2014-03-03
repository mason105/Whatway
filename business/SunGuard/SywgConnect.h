#ifndef SYWG_CONNECT_H
#define SYWG_CONNECT_H




#include <string>

#include <boost/asio.hpp>

#include "business/IBusiness.h"

// 采用同步模式实现
// 采用boost::asio，实现跨平台
// 需要支持CRC
// 需要支持DES加密算法, botan支持
// 连接号和des密钥和具体业务无关
// 要定义业务接口xml

class CSywgConnect : public IBusiness
{
public:
	CSywgConnect(void);
	~CSywgConnect(void);

public:
	virtual bool CreateConnect();
	virtual void CloseConnect();
	virtual bool Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg);
	
	

	



	WORD GetCRC(void* msg, size_t len);
	int ComputeNetPackSize(int nRealSize);
	int ReadMsgHeader(char * buf);

	bool InitConnect();
	bool GetErrorMsg(int nErrCode, std::string& sErrMsg);
	bool Login(std::string& response, int& status, std::string& errCode, std::string& errMsg);
	bool Send(std::string& response, int& status, std::string& errCode, std::string& errMsg);
	
	//BOOL OpenAccount(std::map<std::string, std::string>& reqmap, std::string& response);
	// 705
	//BOOL BankTransfReq(std::map<std::string, std::string>& reqmap, std::string& response);
	

private:
	long cn_id;      	// 网关连接号
	BYTE des_key[8];         // DES密钥(请用“ExpressT”作为密钥des解密)
	std::string branchNo; 

	boost::asio::io_service ios;
	boost::asio::ip::tcp::socket socket;
	std::string m_sIP;
	int m_nPort;

	

public:
	int m_nErrCode;
	std::string m_sErrMsg;
	void SetErrInfo(int nErrCode, std::string sErrMsg="");
	void GetErrInfo(std::string& sErrCode, std::string& sErrMsg);

	
	

private:
	// 保存登录信息，用于自动重连

};

#endif