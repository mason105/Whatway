#include "stdafx.h"



#include <sstream>
#include <iomanip>

#include <boost/crc.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "SywgConnect.h"

#include "sywg.h"
#include "Sywg_CRC_DES.h"

#include "AGC.h"

#include "./output/FileLog.h"
#include "errcode.h"


CSywgConnect::CSywgConnect(void)
	:socket(ios)
{
	// AGC测试环境
	//m_sIP = "192.168.24.36";
	//m_nPort = 19997;
	//m_sIP = "127.0.0.1";
	//m_nPort = 28888;

	SOH = "\x01";

}


CSywgConnect::~CSywgConnect(void)
{
}

// OK
bool CSywgConnect::CreateConnect()
{
	m_sIP = counter->m_sIP;
		m_nPort = counter->m_nPort;

	try
	{
		boost::system::error_code ec;

		boost::asio::ip::tcp::resolver resolver(ios);

		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), m_sIP, boost::lexical_cast<std::string>(m_nPort));

		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, ec);

		boost::asio::connect(socket, iterator, ec);
	
		//socket->non_blocking(false);

		if (ec)
		{
			//gFileLog::instance().Log(ec.message(), m_sLogFile);
			TRACE("连接失败\n");
			return FALSE;
		}

		TRACE("连接成功\n");
		
		return InitConnect();
	}
	catch(std::exception& e)
	{
		//gFileLog::instance().Log(e.what(), m_sLogFile);
		TRACE("连接异常\n", e.what());
		return FALSE;
	}
}

// OK
void CSywgConnect::Close()
{
	boost::system::error_code ec;

	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	
	if (ec)
	{
		
		//gFileLog::instance().Log(ec.message(), m_sLogFile);
	}
	
	socket.close(ec);
}

// 读SWI_BlockHead
// 返回block_size
int CSywgConnect::ReadMsgHeader(char * buf)
{
	SWI_BlockHead msgHeader;
	memset(&msgHeader, 0x00, sizeof(msgHeader));

	int total_bytes = sizeof(SWI_BlockHead);

	size_t bytes = 0; // 一次接收到的字节数据
	int recv_bytes = 0; // 存放累计收到的数据
	int read_bytes = total_bytes - recv_bytes; // 存放剩余需要读取的数据
	
	boost::system::error_code ec;
	

	
	while ( bytes = socket.read_some(boost::asio::buffer(buf + recv_bytes, read_bytes), ec) )
	{
		if (ec)
		{
				//gFileLog::instance().Log("read package body:" + ec.message());
			TRACE("读消息头失败\n");
			return FALSE;
		}

		recv_bytes += bytes;
		//gFileLog::instance().Log("累计接收：" + boost::lexical_cast<std::string>(recv_bytes));
			
		if (recv_bytes >= total_bytes)
			break;

		read_bytes = total_bytes - recv_bytes;
	}

	if (read_bytes <= 0)
	{
		return FALSE;
	}

	memcpy(&msgHeader, buf, sizeof(SWI_BlockHead));	
	TRACE("读返回包头：功能号：%d, 类型：%d\n", msgHeader.function_no, msgHeader.block_type);

	return msgHeader.block_size;
}



// 返回8的倍数
// ok
int CSywgConnect::ComputeNetPackSize(int nRealSize)
{
    int nRes = 0;
    
    if (nRealSize  > 0)
    {
        int n = nRealSize % 8;
        if(n) 
            nRes = ((nRealSize / 8) + 1) * 8;
        else
            nRes = nRealSize;
    }

    return nRes;
}

// 初始化连接，获得cnid以及des key
bool CSywgConnect::InitConnect()
{
	bool bRet = FALSE;
	int nMsgHeaderSize = sizeof(struct SWI_BlockHead);
	int nCRCBegin = 4; // sizeof(block_size) + sizeof(crc)	
	char * pRequestBuf = NULL;
	char * pReturnBuf = NULL;
	char * pResultBuf = NULL;
	boost::system::error_code ec;
	int nPacketSize = 0; // nBlockSize的8的倍数
	int nBlockSize = 0; // 真实大小
	size_t nSendBytes = 0; // 发送的字节
	int nRecvBytes = 0; // 累计接收的字节


	struct SWI_ConnectRequest request;
	memset(&request, 0, sizeof(SWI_ConnectRequest));

	struct SWI_ConnectResponse ret;
	memset(&ret, 0, sizeof(SWI_ConnectResponse));
	

	// 发送请求
	request.method = 2;
	request.entrust_method = '7'; // 需要写在配置文件中
	request.return_cp = 0;
	request.network[0] = 192;
	request.network[1] = 168;
	request.network[2] = 24;
	request.network[3] = 156;
	request.address[0] = 0;
	request.address[1] = 34;
	request.address[2] = 250;
	request.address[3] = 28;
	request.address[4] = 165;
	request.address[5] = 42;
	request.client_major_ver = 3;
	request.client_minor_ver = 24;
	//request.key_length = 0;
	

	request.head.block_size = sizeof(struct SWI_ConnectRequest);
	request.head.block_type = 6;
	request.head.cn_id = 0;
	//request.head.dest_dpt = 0;
	//request.head.function_no = 0;
	//request.head.reserved1 = 0;
	//request.head.reserved2 = 0;
	

	request.head.crc = gSywg.CalCrc(&request.head.block_type, request.head.block_size - nCRCBegin); // 生成CRC检验码

	
	nPacketSize = ComputeNetPackSize(request.head.block_size);
	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);
    memcpy(pRequestBuf, &request, request.head.block_size);
	
	nSendBytes = socket.write_some(boost::asio::buffer(pRequestBuf, nPacketSize), ec);
	if (ec)
	{
		bRet = FALSE;
		goto error;
	}
	
	if (nSendBytes != nPacketSize)
	{
		bRet = FALSE;
		goto error;
	}

	
	

	// 接收响应
	nBlockSize = sizeof(SWI_ConnectResponse);
	nPacketSize = ComputeNetPackSize(nBlockSize);

	pReturnBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN); // extension_buf_len其实没必要
	memset(pReturnBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	// 读消息头
	// 先读消息头，主要是用来判断包的类型是result还是return

	nBlockSize = ReadMsgHeader(pReturnBuf); // 返回的是整个消息的大小，包括消息头和消息内容
	if (nBlockSize <= 0)
	{
		bRet = FALSE;
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = socket.read_some(boost::asio::buffer(pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
		if (ec)
		{
			bRet = FALSE;
			goto error;
		}

		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	// 把缓存区转换成结构体
	memcpy(&ret, pReturnBuf, sizeof(SWI_ConnectResponse));

	
	

	if (ret.return_value < 0)
	{
		TRACE("初始化连接失败，return_value=%d\n", ret.return_value);
		bRet = FALSE;
		goto error;
	}
	
	cn_id = ret.head.cn_id;
	TRACE("====================初始化连接， 返回连接号%d\n", cn_id);

	int nRet = gSywg.desinit(0);
	nRet = gSywg.dessetkey("ExpressT");
	gSywg.dedes((char*)ret.des_key);
	nRet = gSywg.desdone();
	if (nRet != 0)
	{
		bRet = FALSE;
		goto error;
	}

	memset(des_key, 0x00, sizeof(des_key));
	memcpy_s(des_key, sizeof(des_key), ret.des_key, sizeof(ret.des_key));

	bRet = true;
	TRACE("初始化连接成功\n");

error:
	if (pRequestBuf != NULL)
	{
		free(pRequestBuf);
		pRequestBuf = NULL;
	}

	if (pResultBuf != NULL)
	{
		free(pResultBuf);
		pResultBuf = NULL;
	}

	if (pReturnBuf != NULL)
	{
		free(pReturnBuf);
		pReturnBuf = NULL;
	}

	return bRet;
}

// 2013-10-23
bool CSywgConnect::GetErrorMsg(int nReturnStatus, std::string& sErrMsg)
{
	bool bRet = FALSE;
	int nMsgHeaderSize = sizeof(struct SWI_BlockHead);
	int nCRCBegin = 4; // sizeof(block_size) + sizeof(crc)	
	char * pRequestBuf = NULL;
	char * pReturnBuf = NULL;
	char * pResultBuf = NULL;
	boost::system::error_code ec;
	int nPacketSize = 0; // nBlockSize的8的倍数
	int nBlockSize = 0; // 真实大小
	size_t nSendBytes = 0; // 发送的字节
	int nRecvBytes = 0; // 累计接收的字节

	// 发送请求
	struct SWI_ErrorMsgRequest request;
	memset(&request, 0, sizeof(SWI_ErrorMsgRequest));

	struct SWI_ErrorMsgReturn ret;
	memset(&ret, 0, sizeof(SWI_ErrorMsgReturn));

	
	
	request.error_no = nReturnStatus;
	
	request.head.block_type = 1;
	request.head.cn_id = cn_id;
	request.head.function_no = 0x901;
	request.head.block_size = sizeof(struct SWI_ErrorMsgRequest);
	request.head.crc = gSywg.CalCrc(&request.head.block_type, request.head.block_size - nCRCBegin);


	nPacketSize = ComputeNetPackSize(request.head.block_size);
	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);
    memcpy(pRequestBuf, &request, request.head.block_size);
	
	nSendBytes = socket.write_some(boost::asio::buffer(pRequestBuf, nPacketSize), ec);

	if (ec)
	{
		bRet = FALSE;
		goto error;
	}
	
	if (nSendBytes != nPacketSize)
	{
		bRet = FALSE;
		goto error;
	}
	

	// 接收响应
	
	
	nBlockSize = sizeof(SWI_ErrorMsgReturn);
	nPacketSize = ComputeNetPackSize(nBlockSize);

	pReturnBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN); // extension_buf_len其实没必要
	memset(pReturnBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	// 读消息头
	// 先读消息头，主要是用来判断包的类型是result还是return

	nBlockSize = ReadMsgHeader(pReturnBuf); // 返回的是整个消息的大小，包括消息头和消息内容
	if (nBlockSize <= 0)
	{
		bRet = FALSE;
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = socket.read_some(boost::asio::buffer(pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
		if (ec)
		{
			bRet = FALSE;
			goto error;
		}

		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	// 把缓存区转换成结构体
	memcpy(&ret, pReturnBuf, sizeof(SWI_ErrorMsgReturn));
	if (ret.head.function_no != 0x901 && ret.head.block_type != BLOCK_TYPE_RETURN)
	{
		bRet = FALSE;
		goto error;
	}


	if (ret.return_status > 0)
	{
		sErrMsg = ret.error_msg;
		bRet = TRUE;
		TRACE("查询错误信息功能调用成功，错误码：%d, 错误信息：%s\n", nReturnStatus, sErrMsg.c_str());
	}
	else
	{
		sErrMsg = "功能号返回失败";
		bRet = FALSE;
		TRACE("查询错误信息功能调用失败\n");
	}


error:
	if (pRequestBuf != NULL)
	{
		free(pRequestBuf);
		pRequestBuf = NULL;
	}

	if (pResultBuf != NULL)
	{
		free(pResultBuf);
		pResultBuf = NULL;
	}

	if (pReturnBuf != NULL)
	{
		free(pReturnBuf);
		pReturnBuf = NULL;
	}

	return bRet;
}


// 2013-12-03
bool CSywgConnect::Login(std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	std::string sErrMsg = "";
	bool bRet = FALSE;
	int nMsgHeaderSize = sizeof(struct SWI_BlockHead);
	int nCRCBegin = 4; // sizeof(block_size) + sizeof(crc)	
	char * pRequestBuf = NULL;
	char * pReturnBuf = NULL;
	char * pResultBuf = NULL;
	boost::system::error_code ec;
	int nPacketSize = 0; // nBlockSize的8的倍数
	int nBlockSize = 0; // 真实大小
	size_t nSendBytes = 0; // 发送的字节
	int nRecvBytes = 0; // 累计接收的字节

	

	SWI_AccountLoginRequest request;
	memset(&request, 0x00, sizeof(SWI_AccountLoginRequest));


	struct SWI_AccountLoginResult result;

	struct SWI_AccountLoginReturn ret;


	std::string account = reqmap["account"];
	if (account.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}

	memcpy_s(request.account, sizeof(request.account), reqmap["account"].c_str(), reqmap["account"].length());
	std::string branch_no = account.substr(0, 4);

	std::string account_type = reqmap["account_type"];
	if (account_type.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	request.account_type = account_type.at(0);

	std::string flag = reqmap["flag"];
	if (flag.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	long lFlag = boost::lexical_cast<long>(flag);
	request.flag = lFlag;

	std::string note = reqmap["cssweb_hardinfo"];
	if (note.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	memcpy_s(request.note, sizeof(request.note), note.c_str(), note.length());

	std::string productno = reqmap["productno"];
	if (productno.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	memcpy_s(request.productno, sizeof(request.productno), productno.c_str(), productno.length());

	std::string pwd = reqmap["pwd"];
	if (pwd.empty())
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(PARAM_ERROR);
		errMsg = gError::instance().GetErrMsg(PARAM_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	memcpy_s(request.pwd, sizeof(request.pwd), pwd.c_str(), pwd.length());


	request.head.block_size = sizeof(SWI_AccountLoginRequest);
	request.head.block_type = 1; // request
	request.head.cn_id = cn_id;
	
	request.head.dest_dpt = boost::lexical_cast<WORD>(branch_no);
	request.head.function_no = 0x111;

	request.head.crc = gSywg.CalCrc(&request.head.block_type, request.head.block_size - nCRCBegin);
	
	int nRet = gSywg.desinit(0);
	nRet = gSywg.dessetkey((char*)des_key);
	gSywg.endes(request.pwd);
	nRet = gSywg.desdone();
	if (nRet != 0)
	{
		status = 0;
		errCode = boost::lexical_cast<std::string>(BUSI_CRYPT_ERROR);
		errMsg = gError::instance().GetErrMsg(BUSI_CRYPT_ERROR);


		this->GenResponse(response, errCode, errMsg);

		bRet = true;
		return bRet;
	}


	nPacketSize = ComputeNetPackSize(request.head.block_size);
	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);
    memcpy(pRequestBuf, &request, request.head.block_size);
	
	nSendBytes = socket.write_some(boost::asio::buffer(pRequestBuf, nPacketSize), ec);

	if (ec)
	{
		bRet = FALSE;
		SetErrInfo(ec.value(), ec.message());
		goto error;
	}
	
	if (nSendBytes != nPacketSize)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 场景一：返回result，返回return
	// 场景二：返回return

	// begin Result=========================================================================================
	nBlockSize = sizeof(SWI_AccountLoginResult);
	nPacketSize = ComputeNetPackSize(nBlockSize);
	
	pResultBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN);
	

	do
	{
		memset(pResultBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

		// 读消息头
		nBlockSize = ReadMsgHeader(pResultBuf); // 返回的是整个消息的大小，包括消息头和消息内容
		if (nBlockSize <= 0)
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}

		nPacketSize = ComputeNetPackSize(nBlockSize);

		// 读消息内容
		int nRecvBytes = 0;
		
		do 
		{
			int nBytes = 0;
			nBytes = socket.read_some(boost::asio::buffer(pResultBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
			if (ec)
			{
				bRet = FALSE;
				SetErrInfo(ec.value(), ec.message());
				goto error;
			}

			if (nBytes > 0)
			{
				nRecvBytes += nBytes;
			}
			else if (nBytes <= 0)
			{
				bRet = FALSE;
				SetErrInfo(ERR_CODE_NETWORK);
				goto error;
			}
		} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

		// 把缓存区转换成结构体
		memset(&result, 0x00, sizeof(SWI_AccountLoginResult));
		memcpy(&result, pResultBuf, sizeof(SWI_AccountLoginResult));

		if (result.head.block_type == BLOCK_TYPE_RESULT && result.head.function_no == 0x111)
		{
			if (result.row_no == 0xFFFF)
			{
				break;
			}
			else
			{
				std::string temp;

				std::string bankbook_number = boost::lexical_cast<std::string>(result.bankbook_number);
				temp += "bankbook_number=";
				temp += bankbook_number;

				temp += "&account_type=";
				temp += result.account_type;

				temp += "&security_account=";
				temp += result.security_account;

				temp += "\n";
				TRACE("登录返回记录：%s\n", temp.c_str());

			}
		}
		else
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_PACKAGE);
			goto error;
		}
	} while(result.row_no != 0xFFF);
	// end result=========================================================================================


	// 接收Return
	nBlockSize = sizeof(SWI_AccountLoginReturn);
	nPacketSize = ComputeNetPackSize(nBlockSize);

	pReturnBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN); // extension_buf_len其实没必要
	memset(pReturnBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	// 读消息头
	// 先读消息头，主要是用来判断包的类型是result还是return

	nBlockSize = ReadMsgHeader(pReturnBuf); // 返回的是整个消息的大小，包括消息头和消息内容
	if (nBlockSize <= 0)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = socket.read_some(boost::asio::buffer(pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
		if (ec)
		{
			bRet = FALSE;
			SetErrInfo(ec.value(), ec.message());
			goto error;
		}

		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	
	memset(&ret, 0x00, sizeof(SWI_AccountLoginReturn));
	memcpy(&ret, pReturnBuf, sizeof(SWI_AccountLoginReturn));

	if (ret.head.function_no != 0x111 || ret.head.block_type != BLOCK_TYPE_RETURN)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_PACKAGE);
		goto error;
	}

	if (ret.return_status <= 0)
	{
		if (GetErrorMsg(ret.return_status, sErrMsg))
		{
			status = 0;
			errCode = boost::lexical_cast<std::string>(ret.return_status);
			errMsg = sErrMsg;


			this->GenResponse(response, errCode, errMsg);

			bRet = true;
			
		}
	}
	else
	{
		
		response = "1";
		response += SOH;
		response += "13";
		response += SOH;

		

		response += "bankbook_number" + SOH;
		response += "account_status" + SOH;
		response += "name" + SOH;
		response += "id" + SOH;
		response += "card_version" + SOH;
		response += "customer_flag" + SOH;
		response += "Cust_flag" + SOH;
		response += "Cust_risk_type" + SOH;
		response += "depart_number" + SOH;
		response += "last_login_date" + SOH;
		response += "last_login_time" + SOH;
		response += "last_full_note" + SOH;
		response += "reserved_auth_info" + SOH;


		response += boost::lexical_cast<std::string>(ret.bankbook_number) + SOH;
		int account_status = ret.account_status;
		response += boost::lexical_cast<std::string>(account_status) + SOH;
		response += boost::lexical_cast<std::string>(ret.name) + SOH;
		response += boost::lexical_cast<std::string>(ret.id) + SOH;
		int card_version = ret.card_version;
		response += boost::lexical_cast<std::string>(card_version) + SOH;
		response += boost::lexical_cast<std::string>(ret.customer_flag) + SOH;
		response += boost::lexical_cast<std::string>(ret.Cust_flag) + SOH;
		response += boost::lexical_cast<std::string>(ret.Cust_risk_type) + SOH;
		response += boost::lexical_cast<std::string>(ret.depart_number) + SOH;
		response += boost::lexical_cast<std::string>(ret.last_login_date) + SOH;
		response += boost::lexical_cast<std::string>(ret.last_login_time) + SOH;
		response += boost::lexical_cast<std::string>(ret.last_full_note) + SOH;
		response += boost::lexical_cast<std::string>(ret.reserved_auth_info) + SOH;
		

		status = 1;
		bRet = true;
	}

error:
	if (pRequestBuf != NULL)
	{
		free(pRequestBuf);
		pRequestBuf = NULL;
	}

	if (pResultBuf != NULL)
	{
		free(pResultBuf);
		pResultBuf = NULL;
	}

	if (pReturnBuf != NULL)
	{
		free(pReturnBuf);
		pReturnBuf = NULL;
	}

	return bRet;
}

void CSywgConnect::SetErrInfo(int nErrCode, std::string sErrMsg)
{
	m_nErrCode = nErrCode;

	if (!sErrMsg.empty())
	{

		m_sErrMsg = sErrMsg;
	}
	else
	{
		switch (m_nErrCode)
		{
		case ERR_CODE_NETWORK:
			m_sErrMsg = "网络读写错误";
			break;
		case ERR_CODE_DES:
			m_sErrMsg = "des错误";
			break;
		case ERR_CODE_PACKAGE:
			m_sErrMsg = "串包";
			break;
		default:
			m_sErrMsg = "未定义错误";
			break;
		}
	}
}

void CSywgConnect::GetErrInfo(std::string& sErrCode, std::string& sErrMsg)
{
	sErrMsg = m_sErrMsg;

	sErrCode = boost::lexical_cast<std::string>(m_nErrCode);

	
}



bool CSywgConnect::IsConnected()
{
	return socket.is_open();
}


bool CSywgConnect::Send(std::string& request, std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	/*
	分析请求
	如果是登录，就调用登录功能，如果登录成功，就把状态改为已登录
	如果是
	*/
	this->ParseRequest(request);


	bool bRet = false;

	
	if (funcid == "0x111")
	{
		bRet = Login(response, status, errCode, errMsg);
	}
	else
	{
		bRet = Send(response, status, errCode, errMsg);
	}

	

	return bRet;
}

bool CSywgConnect::Send(std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	std::string sErrMsg = "";
	BOOL bRet = FALSE;
	int nMsgHeaderSize = sizeof(struct SWI_BlockHead);
	int nCRCBegin = 4; // sizeof(block_size) + sizeof(crc)	
	char * pRequestBuf = NULL;
	char * pReturnBuf = NULL;
	char * pResultBuf = NULL;
	boost::system::error_code ec;
	int nPacketSize = 0; // nBlockSize的8的倍数
	int nBlockSize = 0; // 真实大小
	size_t nSendBytes = 0; // 发送的字节
	int nRecvBytes = 0; // 累计接收的字节
	std::string branch_no = "";
	WORD row_no = 0;
	long return_status = 0;

	std::string sResult = ""; // 存放result
	int nRows = 0; //结果集行数

	SWI_BlockHead headRequest;

	std::string funcid = reqmap["cssweb_funcid"];
	
	WORD wFuncId = strtol(funcid.c_str(), NULL, 16);

	FUNC func =  gAGC.GetFunctionById(funcid);

	char * request = new char[nMsgHeaderSize + func.sizeRequest]; // 总的大小

	int pos = 0;
	pos += nMsgHeaderSize;

	// 必须按顺序，并且是一字节对齐
	for (std::vector<FIELD>::iterator it = func.vRequest.begin(); it != func.vRequest.end(); it++)
	{
		FIELD field = *it;

		// 得到请求值
		std::string value = reqmap[field.name];

		//如果前端没有传值，那么可以传送默认值

		if (field.name == "account")
			branch_no = value.substr(0, 4);

		if (field.type == "char")
		{
			char * buf = new char[field.size];
			memset(buf, 0x00, field.size);

			// 把请求串中的值copy到buf
			memcpy(buf, value.c_str(), value.size());

			memcpy(request + pos, buf, field.size);

			

			pos += field.size;

			delete[] buf;
		}

		if (field.type == "BYTE")
		{
			BYTE buf;

			if (value.empty())
			{
				buf = '0';
			}
			else
			{
				buf = boost::lexical_cast<BYTE>(value);
			}

			memcpy(request + pos, &buf, sizeof(BYTE));
			pos += sizeof(BYTE);
			
		}

		if (field.type == "WORD")
		{
			WORD buf;

			if (value.empty())
			{
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<WORD>(value);
			}

			memcpy(request + pos, &buf, sizeof(WORD));
			pos += sizeof(WORD);
			
		}

		if (field.type == "int")
		{
			int  buf;
			if (value.empty())
			{
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<int>(value);
			}

			memcpy(request + pos, &buf, sizeof(int));
			pos += sizeof(int);
			
		}

		if (field.type == "long")
		{
			long  buf;
			
			if (value.empty())
			{
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<long>(value);
			}

			memcpy(request + pos, &buf, sizeof(long));
			pos += sizeof(long);
			
		}
		if (field.type == "short")
		{
			short  buf;
			
			if (value.empty())
			{
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<short>(value);
			}

			memcpy(request + pos, &buf, sizeof(short));
			pos += sizeof(short);
			
		}
		if (field.type == "SWISmallMoney")
		{
			long  buf;
			if (value.empty())
			{
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<long>(value);
			}

			memcpy(request + pos, &buf, sizeof(long));
			pos += sizeof(long);
			
		}

		if (field.type == "SWIMoney")
		{
			double buf;
			
			if (value.empty())
			{
				//前端没有传值
				buf = 0;
			}
			else
			{
				buf = boost::lexical_cast<double>(value);
			}

			memcpy(request + pos, &buf, sizeof(double));
			pos += sizeof(double);

			// 这里如何转成SWIMoney类型呢？？？
			//entrustReq.apply_amount = double(strtod(szValue,NULL));
			
		}
	}// end for request

	

	headRequest.block_size = nMsgHeaderSize + func.sizeRequest;
	headRequest.block_type = 1; // request
	headRequest.cn_id = cn_id;
	
	headRequest.dest_dpt = boost::lexical_cast<WORD>(branch_no);
	headRequest.function_no = wFuncId;

	memcpy(request, &headRequest, sizeof(SWI_BlockHead));

	headRequest.crc = gSywg.CalCrc(request + nCRCBegin, headRequest.block_size - nCRCBegin);

	memcpy(request, &headRequest, sizeof(SWI_BlockHead));

	// 处理加密
	// 一直没搞懂，为什么要放在crc之后呢
	pos = nMsgHeaderSize;
	for (std::vector<FIELD>::iterator it = func.vRequest.begin(); it != func.vRequest.end(); it++)
	{
		FIELD field = *it;

		
		if (field.bEncrypt)
		{
			int nRet = gSywg.desinit(0);
			nRet = gSywg.dessetkey((char*)des_key); // 通过初始化返回

			gSywg.endes(request + pos); //security_pwd
			pos += field.size;

			nRet = gSywg.desdone();
			if (nRet != 0)
			{
				status = 0;
				errCode = boost::lexical_cast<std::string>(BUSI_CRYPT_ERROR);
				errMsg = gError::instance().GetErrMsg(BUSI_CRYPT_ERROR);

		
				this->GenResponse(response, errCode, errMsg);

				bRet = true;
				goto error;
			}

			

		}
		else
		{
			if (field.size != 0) // char || BYTE
			{
				pos += field.size;
			}
			else
			{
				if (field.type == "WORD")
					pos += sizeof(WORD);

				if (field.type == "BYTE")
					pos += sizeof(BYTE);

				if (field.type == "long")
					pos += sizeof(long);

				if (field.type == "int")
					pos += sizeof(int);

				if (field.type == "SWIMoney")
					pos += sizeof(SWIMoney);

				if (field.type == "short")
					pos += sizeof(short);

				if (field.type == "SWISmallMoney")
					pos += sizeof(long);

			}
		}
	}
	// 处理加密结束
	

	nPacketSize = ComputeNetPackSize(headRequest.block_size);

	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);

    memcpy(pRequestBuf, request, headRequest.block_size);
	
	nSendBytes = socket.write_some(boost::asio::buffer(pRequestBuf, nPacketSize), ec);

	if (ec)
	{
		bRet = FALSE;
		SetErrInfo(ec.value(), ec.message());
		goto error;
	}
	
	if (nSendBytes != nPacketSize)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}


	// 场景一：返回result，返回return
	// 场景二：返回return

	if (func.response == "return")
		goto RETURN;

	// begin Result=========================================================================================
	nBlockSize = nMsgHeaderSize + func.sizeResult;
	nPacketSize = ComputeNetPackSize(nBlockSize);
	
	pResultBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN);
	memset(pResultBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	do
	{
		

		// 读消息头
		nBlockSize = ReadMsgHeader(pResultBuf); // 返回的是整个消息的大小，包括消息头和消息内容
		if (nBlockSize <= 0)
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}

		nPacketSize = ComputeNetPackSize(nBlockSize);

		// 读消息内容
		int nRecvBytes = 0;
		
		do 
		{
			int nBytes = 0;
			nBytes = socket.read_some(boost::asio::buffer(pResultBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
			if (ec)
			{
				bRet = FALSE;
				SetErrInfo(ec.value(), ec.message());
				goto error;
			}

			if (nBytes > 0)
			{
				nRecvBytes += nBytes;
			}
			else if (nBytes <= 0)
			{
				bRet = FALSE;
				SetErrInfo(ERR_CODE_NETWORK);
				goto error;
			}
		} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

		// 把缓存区转换成结构体
		
		SWI_BlockHead headResult;

		//memset(&result, 0x00, sizeof(SWI_QueryReleatedAccountResult));
		pos = 0;

		memcpy(&headResult, pResultBuf + pos, sizeof(SWI_BlockHead));
		pos += nMsgHeaderSize;

		memcpy(&row_no, pResultBuf + nMsgHeaderSize, sizeof(WORD));
		pos += sizeof(WORD);

		if (headResult.block_type == BLOCK_TYPE_RESULT && headResult.function_no == wFuncId)
		{
			if (row_no == 0xFFFF)
			{
				break;
			}
			else
			{
				for (std::vector<FIELD>::iterator it = func.vResult.begin(); it != func.vResult.end(); it++)
				{
					FIELD field = *it;

					if (field.name == "row_no")
						continue;

					if (field.type == "char")
					{
						char * buf = new char[field.size+1];
						memset(buf, 0x00, field.size+1);

						memcpy(buf, pResultBuf + pos, field.size);
						buf[field.size] = '\0';

						pos += field.size;

						int len = strlen(buf);
						std::string tmp(buf, len);

						sResult += tmp + SOH;
						delete[] buf;
					}

					if (field.type == "BYTE")
					{
						BYTE buf;

						memcpy(&buf, pResultBuf + pos, sizeof(BYTE));
						pos += sizeof(BYTE);

						int val = buf;
						sResult += boost::lexical_cast<std::string>(val) + SOH;

						//int main_flag = result.main_flag;
						//sResult += boost::lexical_cast<std::string>(main_flag) + SOH;

					
					}

					if (field.type == "WORD")
					{
						WORD  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(WORD));
						pos += sizeof(WORD);

						sResult += boost::lexical_cast<std::string>(buf) + SOH;
						
					}

					if (field.type == "long")
					{
						long  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(long));
						pos += sizeof(long);

						sResult += boost::lexical_cast<std::string>(buf) + SOH;
					}

					if (field.type == "short")
					{
						short  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(short));
						pos += sizeof(short);

						sResult += boost::lexical_cast<std::string>(buf) + SOH;
					}

					if (field.type == "SWISmallMoney")
					{
						long  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(long));
						pos += sizeof(long);

						sResult += boost::lexical_cast<std::string>(buf) + SOH;
					}

					if (field.type == "int")
					{
						int  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(int));
						pos += sizeof(int);

						sResult += boost::lexical_cast<std::string>(buf) + SOH;
					}

					if (field.type == "SWIMoney")
					{
						__int64  buf;

						memcpy(&buf, pResultBuf + pos, sizeof(__int64));
						pos += sizeof(__int64);

						double val = buf/10000.0;

						std::ostringstream osbuf2;    
						osbuf2 << std::setiosflags(std::ios::fixed) << std::setprecision(2);
						osbuf2 << val;
						
						std::string tmp = osbuf2.str();
						

						sResult += tmp + SOH;
					}
					

				}// end for vResult


				nRows++;

			}
		}
		else
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_PACKAGE);
			goto error;
		}
	} while(row_no != 0xFFF);
	// end result=========================================================================================

	

RETURN:
	// 接收Return
	nBlockSize = nMsgHeaderSize + func.sizeReturn;
	nPacketSize = ComputeNetPackSize(nBlockSize);

	pReturnBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN); // extension_buf_len其实没必要
	memset(pReturnBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	// 读消息头
	// 先读消息头，主要是用来判断包的类型是result还是return

	nBlockSize = ReadMsgHeader(pReturnBuf); // 返回的是整个消息的大小，包括消息头和消息内容
	if (nBlockSize <= 0)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = socket.read_some(boost::asio::buffer(pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes), ec);
		if (ec)
		{
			bRet = FALSE;
			SetErrInfo(ec.value(), ec.message());
			goto error;
		}

		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	SWI_BlockHead headReturn;

	//memset(&result, 0x00, sizeof(SWI_QueryReleatedAccountResult));
	pos = 0;

	memcpy(&headReturn, pReturnBuf + pos, sizeof(SWI_BlockHead));
	pos += nMsgHeaderSize;

	if (headReturn.function_no != wFuncId || headReturn.block_type != BLOCK_TYPE_RETURN)
	{
		bRet = FALSE;
		SetErrInfo(ERR_CODE_PACKAGE);
		goto error;
	}

	memcpy(&return_status, pReturnBuf + pos, sizeof(long));
	pos += sizeof(long);

	if (return_status <= 0)
	{
		

		if (GetErrorMsg(return_status, sErrMsg))
		{
			for (std::vector<FIELD>::iterator it = func.vReturn.begin(); it != func.vReturn.end(); it++)
			{
				FIELD field = *it;

				if (field.name == "return_status")
					continue;

				if (field.name == "errmsg")
				{
					
					char * buf = new char[field.size+1];
					memset(buf, 0x00, field.size+1);

					memcpy(buf, pReturnBuf + pos, field.size);
					buf[field.size] = '\0';

					pos += field.size;

					int len = strlen(buf);
					std::string tmp(buf, len);

					sErrMsg = tmp;
					delete[] buf;
				}
				else
				{
					if (field.size != 0) // char || BYTE
					{
						pos += field.size;
					}
					else
					{
						if (field.type == "WORD")
							pos += sizeof(WORD);

						if (field.type == "BYTE")
							pos += sizeof(BYTE);

						if (field.type == "long")
							pos += sizeof(long);

						if (field.type == "int")
							pos += sizeof(int);

						if (field.type == "SWIMoney")
							pos += sizeof(SWIMoney);

						if (field.type == "short")
							pos += sizeof(short);

						if (field.type == "SWISmallMoney")
							pos += sizeof(long);

					}
				}
			}// end for vReturn

			
			status = 0;
			errCode = boost::lexical_cast<std::string>(return_status);
			errMsg = sErrMsg;


			this->GenResponse(response, errCode, errMsg);

			bRet = true;

			goto error;
		}
	}
	else
	{
		// 处理成功

		if (func.response == "result")
		{
			// 如果请求返回result
			// 添加列头
			response = boost::lexical_cast<std::string>(nRows);
			response += SOH;
			int cols = func.vResult.size() - 1; // 减1是因为row_no
			response += boost::lexical_cast<std::string>(cols);
			response += SOH;

			for (std::vector<FIELD>::iterator it = func.vResult.begin(); it != func.vResult.end(); it++)
			{
				FIELD field = *it;

				if (field.name == "row_no")
					continue;

				response += field.name + SOH;
			}

		
			// 添加内容
			response += sResult;
		}
		else
		{
			// 处理return类型
			response = "1";
			response += SOH;

			int cols = func.vReturn.size(); // 
			response += boost::lexical_cast<std::string>(cols);
			response += SOH;

			for (std::vector<FIELD>::iterator it = func.vReturn.begin(); it != func.vReturn.end(); it++)
			{
				FIELD field = *it;
				response += field.name + SOH;
			}
			//
			response += boost::lexical_cast<std::string>(return_status) + SOH;

			for (std::vector<FIELD>::iterator it = func.vReturn.begin(); it != func.vReturn.end(); it++)
			{
				FIELD field = *it;

				if (field.name == "return_status")
					continue;

				if (field.type == "char")
				{
					char * buf = new char[field.size+1];
					memset(buf, 0x00, field.size+1);

					memcpy(buf, pReturnBuf + pos, field.size);
					buf[field.size] = '\0';

					pos += field.size;

					int len = strlen(buf);
					std::string tmp(buf, len);

					response += tmp + SOH;
					delete[] buf;
				}

				if (field.type == "BYTE")
				{
					BYTE buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(BYTE));
					pos += sizeof(BYTE);

					int val = buf;
					response += boost::lexical_cast<std::string>(val) + SOH;
				}

				if (field.type == "WORD")
				{
					WORD  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(WORD));
					pos += sizeof(WORD);

					response += boost::lexical_cast<std::string>(buf) + SOH;
				}

				if (field.type == "long")
				{
					long  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(long));
					pos += sizeof(long);

					response += boost::lexical_cast<std::string>(buf) + SOH;
				}

				if (field.type == "SWISmallMoney")
				{
					long  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(long));
					pos += sizeof(long);

					response += boost::lexical_cast<std::string>(buf) + SOH;
				}

				if (field.type == "short")
				{
					short  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(short));
					pos += sizeof(short);

					response += boost::lexical_cast<std::string>(buf) + SOH;
				}

				if (field.type == "int")
				{
					int  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(int));
					pos += sizeof(int);

					response += boost::lexical_cast<std::string>(buf) + SOH;
				}

				if (field.type == "SWIMoney")
				{
					__int64  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(__int64));
					pos += sizeof(__int64);

					double val = buf/10000.0;

					std::ostringstream osbuf2;    
					osbuf2 << std::setiosflags(std::ios::fixed) << std::setprecision(2);
					osbuf2 << val;
						
					std::string tmp = osbuf2.str();
						

					response += tmp + SOH;
				}
			}// end for vReturn
		}

		
		bRet = true;

		status = 1;
	}

error:
	

	if (pRequestBuf != NULL)
	{
		free(pRequestBuf);
		pRequestBuf = NULL;
	}

	if (pResultBuf != NULL)
	{
		free(pResultBuf);
		pResultBuf = NULL;
	}

	if (pReturnBuf != NULL)
	{
		free(pReturnBuf);
		pReturnBuf = NULL;
	}

	return bRet;
	
}
