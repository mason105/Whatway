#include "stdafx.h"



#include <sstream>
#include <iomanip>

#include <boost/crc.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "SywgConnect.h"


#include "Sywg_CRC_DES.h"

#include "AGC.h"

#include "./output/FileLog.h"
#include "errcode.h"


CSywgConnect::CSywgConnect(void)
{
	// AGC测试环境
	//m_sIP = "192.168.24.36";
	//m_nPort = 19997;
	//m_sIP = "127.0.0.1";
	//m_nPort = 28888;

	

}


CSywgConnect::~CSywgConnect(void)
{
	
}

// OK
bool CSywgConnect::CreateConnect()
{
	int rc = 0;
	u_long bio = 1;
	int connectTimeout = m_Counter->m_nConnectTimeout;
	int readTimeout = m_Counter->m_nRecvTimeout * 1000;
	int writeTimeout = m_Counter->m_nRecvTimeout * 1000;
	sockfd = INVALID_SOCKET;


	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET )
	{
		return FALSE;
	}
	
	//设置为非阻塞模式
	bio = 1;
	rc = ioctlsocket(sockfd, FIONBIO, &bio); 
	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_Counter->m_sIP.c_str());
	addr.sin_port = htons(m_Counter->m_nPort);

	rc = connect(sockfd, (const sockaddr *)&addr, sizeof(addr));
	// 异步模式不用判断
	if (rc == SOCKET_ERROR)
	{
		//closesocket(sockfd);
		//return FALSE;
	}

	
	
	
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);

	struct timeval timeout;
	timeout.tv_sec = connectTimeout;
	timeout.tv_usec = 0;

	rc = select(0, NULL, &writefds, NULL, &timeout);
	if (rc == 0)
	{
		// timeout
		closesocket(sockfd);
		return FALSE;
	}

	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}

	if(!FD_ISSET(sockfd, &writefds))  
    {  
		closesocket(sockfd);
		return FALSE;
    }  

	 
	// 设置为阻塞模式
	bio = 0;
	rc = ioctlsocket(sockfd, FIONBIO, &bio);
	if (rc == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return FALSE;
	}

	// 设置读写超时
	//rc = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&readTimeout, sizeof(readTimeout));
    //rc = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(char*)&writeTimeout, sizeof(writeTimeout));

	
		
	m_bConnected =  InitConnect();
	return m_bConnected;
	
}

// OK
void CSywgConnect::CloseConnect()
{
	closesocket(sockfd);
	sockfd = INVALID_SOCKET;
	
	m_bConnected = false;
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
	

	if ( !Recv(buf, total_bytes, 0))
	{
		return FALSE;
	}

	/*
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
	*/

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
	
	if (!Send(pRequestBuf, nPacketSize, 0))
	{
		bRet = FALSE;
		goto error;
	}
	/*
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
	*/

	
	

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
		nBytes = recv(sockfd, pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
		

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
	
	if (!Send(pRequestBuf, nPacketSize, 0))
	{
		bRet = FALSE;
		goto error;
	}

	/*
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
	*/

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
		nBytes = recv(sockfd, pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
		

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
		this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);



		bRet = true;
		return bRet;
	}

	memcpy_s(request.account, sizeof(request.account), reqmap["account"].c_str(), reqmap["account"].length());
	branchNo = account.substr(0, 4);

	std::string account_type = reqmap["account_type"];
	if (account_type.empty())
	{
		this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	request.account_type = account_type.at(0);

	std::string flag = reqmap["flag"];
	if (flag.empty())
	{
		

		GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	long lFlag = boost::lexical_cast<long>(flag);
	request.flag = lFlag;

	std::string note = reqmap["cssweb_hardinfo"];
	if (note.empty())
	{
		//GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

		//bRet = true;
		//return bRet;
	}
	else
	{
		memcpy_s(request.note, sizeof(request.note), note.c_str(), note.length());
	}

	std::string productno = reqmap["productno"];
	if (productno.empty())
	{
		GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	memcpy_s(request.productno, sizeof(request.productno), productno.c_str(), productno.length());

	std::string pwd = reqmap["pwd"];
	if (pwd.empty())
	{
		GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

		bRet = true;
		return bRet;
	}
	memcpy_s(request.pwd, sizeof(request.pwd), pwd.c_str(), pwd.length());


	request.head.block_size = sizeof(SWI_AccountLoginRequest);
	request.head.block_type = 1; // request
	request.head.cn_id = cn_id;
	
	request.head.dest_dpt = boost::lexical_cast<WORD>(branchNo);
	request.head.function_no = 0x111;

	request.head.crc = gSywg.CalCrc(&request.head.block_type, request.head.block_size - nCRCBegin);
	
	int nRet = gSywg.desinit(0);
	nRet = gSywg.dessetkey((char*)des_key);
	gSywg.endes(request.pwd);
	nRet = gSywg.desdone();
	if (nRet != 0)
	{
		GenResponse(BUSI_CRYPT_ERROR, gError::instance().GetErrMsg(BUSI_CRYPT_ERROR), response, status, errCode, errMsg);
		

		bRet = true;
		return bRet;
	}


	nPacketSize = ComputeNetPackSize(request.head.block_size);
	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);
    memcpy(pRequestBuf, &request, request.head.block_size);
	
	if (!Send(pRequestBuf, nPacketSize, 0))
	{
		bRet = FALSE;
		goto error;
	}
	/*
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
	*/

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
//			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}

		nPacketSize = ComputeNetPackSize(nBlockSize);

		// 读消息内容
		int nRecvBytes = 0;
		
		do 
		{
			int nBytes = 0;
			nBytes = recv(sockfd, pResultBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
			

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
//			SetErrInfo(ERR_CODE_PACKAGE);
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
//		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = recv(sockfd, pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
		
		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			//SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	
	memset(&ret, 0x00, sizeof(SWI_AccountLoginReturn));
	memcpy(&ret, pReturnBuf, sizeof(SWI_AccountLoginReturn));

	if (ret.head.function_no != 0x111 || ret.head.block_type != BLOCK_TYPE_RETURN)
	{
		bRet = FALSE;
		//SetErrInfo(ERR_CODE_PACKAGE);
		goto error;
	}

	if (ret.return_status <= 0)
	{
		if (GetErrorMsg(ret.return_status, sErrMsg))
		{
			GenResponse(ret.return_status, sErrMsg, response, status, errCode, errMsg);
			

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
/*
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
*/

/*

bool CSywgConnect::IsConnected()
{
	return socket.is_open();
}
*/

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
	/*
	else if(funcid == "0x4603")
	{
		bRet = f4603(response, status, errCode, errMsg);
	}
	
	else if(funcid == "0x4605")
	{
		bRet = f4605(response, status, errCode, errMsg);
	}
	else if(funcid == "0x4606")
	{
		bRet = f4606(response, status, errCode, errMsg);
	}
	*/
	else
	{
		bRet = Send(response, status, errCode, errMsg);
	}

	

	return bRet;
}

bool CSywgConnect::Send(std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	bool bRet = true;
	std::string sErrMsg = "";
	
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

	WORD row_no = 0;
	long return_status = 0;

	std::string sResult = ""; // 存放result
	int nRows = 0; //结果集行数

	SWI_BlockHead headRequest;

	if (branchNo.empty())
	{
		this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);
		bRet = true;
		return bRet;
	}

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

		//if (field.name == "account")
		//	branch_no = value.substr(0, 4);

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
			long  buf = GetSmallMoney(value);
			//if (value.empty())
			//{
			//	buf = 0;
			//}
			//else
			//{
			//	buf = boost::lexical_cast<long>(value);
			//}

			memcpy(request + pos, &buf, sizeof(long));
			pos += sizeof(long);
			
		}

		if (field.type == "SWIMoney")
		{
			int64_t money = GetMoney(value);

			
			
			memcpy(request + pos, &money, sizeof(long long));
			pos += sizeof(long long);

			// 这里如何转成SWIMoney类型呢？？？
			//entrustReq.apply_amount = double(strtod(szValue,NULL));
			
		}
	}// end for request

	

	headRequest.block_size = nMsgHeaderSize + func.sizeRequest;
	headRequest.block_type = 1; // request
	headRequest.cn_id = cn_id;
	
	if (!branchNo.empty())
		headRequest.dest_dpt = boost::lexical_cast<WORD>(branchNo);

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
				
				errMsg = gError::instance().GetErrMsg(BUSI_CRYPT_ERROR);

		
				GenResponse(BUSI_CRYPT_ERROR, errMsg, response, status, errCode, errMsg);

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
					pos += sizeof(int64_t);

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
	
	if (!Send(pRequestBuf, nPacketSize, 0))
	{
		bRet = FALSE;
		goto error;
	}
	/*
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
	*/

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
//			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}

		nPacketSize = ComputeNetPackSize(nBlockSize);

		// 读消息内容
		int nRecvBytes = 0;
		
		do 
		{
			int nBytes = 0;
			nBytes = recv(sockfd, pResultBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
			

			if (nBytes > 0)
			{
				nRecvBytes += nBytes;
			}
			else if (nBytes <= 0)
			{
				bRet = FALSE;
				//SetErrInfo(ERR_CODE_NETWORK);
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

						
						sResult += GetSmallMoney(buf) + SOH;
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
						//SWIMoney buf;
						int64_t buf;
						
						//char buf[8];
						int s = sizeof(SWIMoney);
						
						

						memcpy(&buf, pResultBuf + pos, s);
						pos += sizeof(SWIMoney);


						//int64_t t = read_le_dd(&buf);
						
						//Int64_char(szBuf, buf, 1);

						//long t1 = ntohl(buf.hi_value);
						//long t2 = ntohl(buf.lo_value);
						///long t3 = htonl(buf.hi_value);
						// t4 = htonl(buf.lo_value);

						// 方式1
						//double tmp = double(buf);

						// 方式2
						//double tmp2 = double(buf);
						

						sResult += GetMoney(buf) + SOH;
					}
					

				}// end for vResult


				nRows++;

			}
		}
		else
		{
			bRet = FALSE;
//			SetErrInfo(ERR_CODE_PACKAGE);
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
//		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = recv(sockfd, pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
		

		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			//SetErrInfo(ERR_CODE_NETWORK);
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
		//SetErrInfo(ERR_CODE_PACKAGE);
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
							pos += sizeof(int64_t);

						if (field.type == "short")
							pos += sizeof(short);

						if (field.type == "SWISmallMoney")
							pos += sizeof(long);

					}
				}
			}// end for vReturn

			
			

			GenResponse(return_status, sErrMsg, response, status, errCode, errMsg);

			bRet = true;

			goto error;
		}
	}
	else
	{
		// 处理成功

		if (func.response == "result")
		{
			if (nRows == 0)
			{
				// 操作成功，柜台没有数据返回
				RetNoRecordRes(response, status);
			}
			else
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

					response += GetSmallMoney(buf) + SOH;
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
					int64_t  buf;

					memcpy(&buf, pReturnBuf + pos, sizeof(int64_t));
					pos += sizeof(int64_t);

					
						

					response += GetMoney(buf) + SOH;
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

int64_t CSywgConnect::GetMoney(std::string buf)
{
	int64_t ret = 0;
	std::string result = "0";
	std::string integer = "";
	std::string decimal = "";

	std::vector<std::string> kv;
	boost::split(kv, buf, boost::is_any_of("."));

	// 没有小数点
	if (kv.size() == 1)
	{
		integer = kv[0];
		decimal = "0000";
		result = integer + decimal;
	}

	if (kv.size() == 2)
	{
		integer = kv[0];

		decimal = kv[1];
		for (int i = decimal.length(); i < 4; i++)
		{
			decimal += "0";
		}

		result = integer + decimal;
	}

	
	ret = _atoi64(result.c_str());
	return ret;
}

long CSywgConnect::GetSmallMoney(std::string buf)
{
	long ret = 0;
	std::string result = "0";
	std::string integer = "";
	std::string decimal = "";

	std::vector<std::string> kv;
	boost::split(kv, buf, boost::is_any_of("."));

	// 没有小数点
	if (kv.size() == 1)
	{
		integer = kv[0];
		decimal = "000";
		result = integer + decimal;
	}

	if (kv.size() == 2)
	{
		integer = kv[0];

		decimal = kv[1];
		for (int i = decimal.length(); i < 3; i++)
		{
			decimal += "0";
		}

		result = integer + decimal;
	}

	
	ret = atol(result.c_str());
	return ret;
}

std::string CSywgConnect::GetMoney(int64_t money)
{
	double val = money/10000.0;

	std::ostringstream osbuf2;    
	osbuf2 << std::setiosflags(std::ios::fixed) << std::setprecision(2);
	osbuf2 << val;
						
	return osbuf2.str();
}

std::string CSywgConnect::GetSmallMoney(long price)
{
	double val = price/1000.0;

	std::ostringstream osbuf2;    
	osbuf2 << std::setiosflags(std::ios::fixed) << std::setprecision(2);
	osbuf2 << val;
						
	return osbuf2.str();
}

int CSywgConnect::Send(const char * buf, int len, int flags)
{
	int rc = 0;
	
	int totalBytes = 0;

	do
	{
		int bytes = 0;
		bytes = send(sockfd, buf + totalBytes, len - totalBytes, flags);
		if (bytes == 0)
		{
			// connection is closed
			break;
		}

		if (bytes < 0)
		{
			break;
		}

		totalBytes += bytes;


	} while(totalBytes < len);

	if (totalBytes != len)
		rc = FALSE;
	else
		rc = TRUE;

	return rc;
}

int CSywgConnect::Recv(char* buf, int len, int flags)
{
	int rc = 0;
	int totalBytes = 0;

	do
	{
		int bytes = 0;
		bytes = recv(sockfd, buf + totalBytes, len - totalBytes, flags);
		if (bytes == 0)
		{
			// connection is closed
			break;
		}

		if (bytes < 0)
		{
			break;
		}

		totalBytes += bytes;


	} while(totalBytes < len);

	if (totalBytes != len)
		rc = FALSE;
	else
		rc = TRUE;

	return rc;
}


bool CSywgConnect::f4603(std::string& response, int& status, std::string& errCode, std::string& errMsg)
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
	std::string sResult = "";

	

	SWI_QueryOTCPDEntrustRequest request;
	memset(&request, 0x00, sizeof(SWI_QueryOTCPDEntrustRequest));


	struct SWI_QueryOTCPDEntrustResult result;

	struct SWI_QueryOTCPDEntrustReturn ret;


	std::string account = reqmap["account"];
	if (account.empty())
	{
		this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);



		bRet = true;
		return bRet;
	}

	memcpy_s(request.account, sizeof(request.account), reqmap["account"].c_str(), reqmap["account"].length());
	branchNo = account.substr(0, 4);

	std::string query_type = reqmap["query_type"];
	//if (query_type.empty())
	//{
	//	this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

	////	bRet = true;
	//	return bRet;
	//}
	request.query_type = query_type.at(0);

	std::string begin_date = reqmap["begin_date"];
	//if (begin_date.empty())
	//{
	//	GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

	//	bRet = true;
	///	return bRet;
	//}
	long lbegin_date = boost::lexical_cast<long>(begin_date);
	request.begin_date = lbegin_date;

	std::string end_date = reqmap["end_date"];
	request.begin_date = boost::lexical_cast<long>(end_date);

	std::string inst_id = reqmap["inst_id"];
	//if (inst_id.empty())
	//{
	//	GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

	//	bRet = true;
	//	return bRet;
	//}
	memcpy_s(request.inst_id, sizeof(request.inst_id), inst_id.c_str(), inst_id.length());

	std::string query_flag = reqmap["query_flag"];
	//if (query_type.empty())
	//{
	//	this->GenResponse(PARAM_ERROR, gError::instance().GetErrMsg(PARAM_ERROR), response, status, errCode, errMsg);

	////	bRet = true;
	//	return bRet;
	//}
	request.query_flag = query_flag.at(0);

	


	request.head.block_size = sizeof(SWI_QueryOTCPDEntrustRequest);
	request.head.block_type = 1; // request
	request.head.cn_id = cn_id;
	
	request.head.dest_dpt = boost::lexical_cast<WORD>(branchNo);
	request.head.function_no = 0x4603;

	request.head.crc = gSywg.CalCrc(&request.head.block_type, request.head.block_size - nCRCBegin);
	
	/*
	int nRet = gSywg.desinit(0);
	nRet = gSywg.dessetkey((char*)des_key);
	gSywg.endes(request.pwd);
	nRet = gSywg.desdone();
	if (nRet != 0)
	{
		GenResponse(BUSI_CRYPT_ERROR, gError::instance().GetErrMsg(BUSI_CRYPT_ERROR), response, status, errCode, errMsg);
		

		bRet = true;
		return bRet;
	}
	*/


	nPacketSize = ComputeNetPackSize(request.head.block_size);
	pRequestBuf = (char*) malloc(nPacketSize);
    memset(pRequestBuf, 0x00, nPacketSize);
    memcpy(pRequestBuf, &request, request.head.block_size);
	
	if (!Send(pRequestBuf, nPacketSize, 0))
	{
		bRet = FALSE;
		goto error;
	}
	/*
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
	*/

	// 场景一：返回result，返回return
	// 场景二：返回return

	// begin Result=========================================================================================
	nBlockSize = sizeof(SWI_QueryOTCPDEntrustResult);
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
//			SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}

		nPacketSize = ComputeNetPackSize(nBlockSize);

		// 读消息内容
		int nRecvBytes = 0;
		
		do 
		{
			int nBytes = 0;
			nBytes = recv(sockfd, pResultBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
			

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
		memset(&result, 0x00, sizeof(SWI_QueryOTCPDEntrustResult));
		memcpy(&result, pResultBuf, sizeof(SWI_QueryOTCPDEntrustResult));

		if (result.head.block_type == BLOCK_TYPE_RESULT && result.head.function_no == 0x4603)
		{
			if (result.row_no == 0xFFFF)
			{
				break;
			}
			else
			{
				char szBuf[50];
				int nDecimal,nSign;

				std::string entrust_date = boost::lexical_cast<std::string>(result.entrust_date);
				sResult += entrust_date;
				sResult += SOH;
				TRACE("entrust_date=%s\n", entrust_date.c_str());

				std::string entrust_sn = result.entrust_sn;
				sResult += entrust_sn;
				sResult += SOH;
				TRACE("entrust_sn=%s\n", entrust_sn.c_str());

				std::string entrust_time = result.entrust_time;
				sResult += entrust_time;
				sResult += SOH;
				TRACE("entrust_time=%s\n", entrust_time.c_str());

				std::string ta_acct = result.ta_acct;
				sResult += ta_acct;
				sResult += SOH;
				TRACE("ta_acct=%s\n", ta_acct.c_str());

				std::string inst_id = result.inst_id;
				sResult += inst_id;
				sResult += SOH;
				TRACE("inst_id=%s\n", inst_id.c_str());

				std::string security_name = result.security_name;
				sResult += security_name;
				sResult += SOH;
				TRACE("security_name=%s\n", security_name.c_str());

				memset(szBuf, 0x00, sizeof(szBuf));
				int len = strlen(result.bs_type);
				memcpy_s(szBuf, 3, result.bs_type, 3);
				//std::string bs_type = result.bs_type;
				sResult += szBuf;
				sResult += SOH;
				TRACE("bs_type=%s\n", szBuf);

				

				memset(szBuf, 0x00, sizeof(szBuf));
				//double app_amt = Int64_double(result.app_amt);
				double app_amt = double(result.app_amt);
				_fcvt_s(szBuf,sizeof(szBuf),app_amt,4,&nDecimal,&nSign);
				int64_t tmp1 = ntohll(app_amt);
				int64_t tmp2 = htonll(app_amt);
				sResult += szBuf;
				sResult += SOH;
				TRACE("app_amt=%s\n", szBuf);

				memset(szBuf, 0x00, sizeof(szBuf));
				double redeem_num = result.redeem_num;
				_fcvt_s(szBuf, sizeof(szBuf), redeem_num, 4, &nDecimal, &nSign);
				sResult += szBuf;
				sResult += SOH;
				TRACE("redeem_num=%s\n", szBuf);

				//std::string entrust_status = result.entrust_status;
				sResult += result.entrust_status;
				sResult += SOH;
				TRACE("entrust_status=%c\n", result.entrust_status);

				//std::string cancel_flag = result.cancel_flag;
				sResult += result.cancel_flag;
				sResult += SOH;
				TRACE("cancel_flag=%c\n", result.cancel_flag);

				std::string orient_app_no = result.orient_app_no;
				sResult += orient_app_no;
				sResult += SOH;
				TRACE("orient_app_no=%s\n", orient_app_no.c_str());

				//std::string cancelled_flag = result.cancelled_flag;
				sResult += result.cancelled_flag;
				sResult += SOH;
				TRACE("cancelled_flag=%c\n", result.cancelled_flag);

				//std::string can_cancel_flag = result.can_cancel_flag;
				sResult += result.can_cancel_flag;
				sResult += SOH;
				TRACE("can_cancel_flag=%c\n", result.can_cancel_flag);

				memset(szBuf, 0x00, sizeof(szBuf));
				double cfm_amt = result.cfm_amt;
				_fcvt_s(szBuf, sizeof(szBuf), cfm_amt, 4, &nDecimal, &nSign);
				sResult += szBuf;
				sResult += SOH;
				TRACE("cfm_amt=%s\n", szBuf);

				memset(szBuf, 0x00, sizeof(szBuf));
				double cfm_num = result.cfm_num;
				_fcvt_s(szBuf, sizeof(szBuf), cfm_num, 4, &nDecimal, &nSign);
				sResult += szBuf;
				TRACE("cfm_num=%s\n", szBuf);
				sResult += SOH;
			}
		}
		else
		{
			bRet = FALSE;
//			SetErrInfo(ERR_CODE_PACKAGE);
			goto error;
		}
	} while(result.row_no != 0xFFF);
	// end result=========================================================================================


	// 接收Return
	nBlockSize = sizeof(SWI_QueryOTCPDEntrustReturn);
	nPacketSize = ComputeNetPackSize(nBlockSize);

	pReturnBuf = (char*)malloc(nPacketSize + EXTENSION_BUF_LEN); // extension_buf_len其实没必要
	memset(pReturnBuf, 0x00, nPacketSize + EXTENSION_BUF_LEN);

	// 读消息头
	// 先读消息头，主要是用来判断包的类型是result还是return

	nBlockSize = ReadMsgHeader(pReturnBuf); // 返回的是整个消息的大小，包括消息头和消息内容
	if (nBlockSize <= 0)
	{
		bRet = FALSE;
//		SetErrInfo(ERR_CODE_NETWORK);
		goto error;
	}

	// 如果返回的类型不变，那么这里nPacketSize应该等于前面的nPakcetSize
	nPacketSize = ComputeNetPackSize(nBlockSize);

	// 读消息内容
	
	
	do 
	{
		int nBytes = 0;
		nBytes = recv(sockfd, pReturnBuf + nMsgHeaderSize + nRecvBytes, nPacketSize - nMsgHeaderSize - nRecvBytes, 0);
		
		if (nBytes > 0)
		{
			nRecvBytes += nBytes;
		}
		else if (nBytes <= 0)
		{
			bRet = FALSE;
			//SetErrInfo(ERR_CODE_NETWORK);
			goto error;
		}
	} while(nRecvBytes < nPacketSize - nMsgHeaderSize);

	
	memset(&ret, 0x00, sizeof(SWI_QueryOTCPDEntrustReturn));
	memcpy(&ret, pReturnBuf, sizeof(SWI_QueryOTCPDEntrustReturn));

	if (ret.head.function_no != 0x4603 || ret.head.block_type != BLOCK_TYPE_RETURN)
	{
		bRet = FALSE;
		//SetErrInfo(ERR_CODE_PACKAGE);
		goto error;
	}

	if (ret.return_status <= 0)
	{
		if (GetErrorMsg(ret.return_status, sErrMsg))
		{
			GenResponse(ret.return_status, sErrMsg, response, status, errCode, errMsg);
			

			bRet = true;
			
		}
	}
	else
	{
		
		response = "1";
		response += SOH;
		response += "16";
		response += SOH;

		

		response += "entrust_date" + SOH;
		response += "entrust_sn" + SOH;
		response += "entrust_time" + SOH;
		response += "ta_acct" + SOH;
		response += "inst_id" + SOH;
		response += "security_name" + SOH;
		response += "bs_type" + SOH;
		response += "app_amt" + SOH;
		response += "redeem_num" + SOH;
		response += "entrust_status" + SOH;
		response += "cancel_flag" + SOH;
		response += "orient_app_no" + SOH;
		response += "cancelled_flag" + SOH;
		response += "can_cancel_flag" + SOH;
		response += "cfm_amt" + SOH;
		response += "cfm_num" + SOH;


		response += sResult;
		
		

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

bool CSywgConnect::f4605(std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	return false;
}

bool CSywgConnect::f4606(std::string& response, int& status, std::string& errCode, std::string& errMsg)
{
	return false;
}

void CSywgConnect::Int64_double(double *l, SWIMoney SW_Money)
{
	*l = (double) (*(__int64 *)(&SW_Money)/10000.); 
}

double CSywgConnect::Int64_double( SWIMoney SW_Money)
{
	return (double) (*(__int64 *)(&SW_Money)/10000.); 
}

/*int64到字符串转换*/
void CSywgConnect::Int64_char(char * buff, SWIMoney SW_Money, BYTE Is_JgdOpen)
{
	/*__int64 m1 = *(__int64 *)(&SW_Money)/10000 ; 
	__int64	m2 = *(__int64 *)(&SW_Money)%10000 ; 

	long j = sprintf( buff, "%ld.", m1 );
	j += sprintf( buff+j, "%d", m2 );*/
	double m = *(__int64 *)(&SW_Money)/10000. ; 
	if( Is_JgdOpen)
		sprintf( buff, "%.4lf", m );
	else
		sprintf( buff, "%.2lf", m );

}

/*数字字符串到int64转换：如、11.12－》111200*/
void CSywgConnect::char_Int64(SWIMoney *SW_Money, const char * QL_Char)
{
#define W_MONEY 8 

	UINT	i;
	char temp[W_MONEY];
	memset( temp, 0, W_MONEY );
	
	for (i=0; (QL_Char[i]!='.')&&(i<strlen(QL_Char)); i++)
	{
		temp[i] = QL_Char[i];
	}
	temp[i+1] = 0;
	double hi = atof( temp );

	UINT j = 0;
	if( i!= strlen(QL_Char))  //有小数部分
	{
		for(j=0; i<strlen(QL_Char); i++,j++ )
			temp[j] = QL_Char[i+1];

		j--;
		for(;j<4;j++)   //小数部分后补零
			temp[j] = '0';
		temp[4] = 0;
	}
	else
		temp[0] = 0;
	
	long lo = atol(temp);

	__int64 qlmoney = __int64 (hi*10000 + lo);
	*(__int64 *)SW_Money = qlmoney;	
}

uint64_t CSywgConnect::read_be_dd(const void* p)  
{  
    const uint8_t* pb = (const uint8_t*)p;  
    pb += 7;  
    uint64_t r;  
    uint8_t* pd = (uint8_t*)&r;  
    for(int i=0; i<8; ++i) {  
        *pd++ = *pb--;  
    }  
    return r;  
}  
uint64_t CSywgConnect::read_le_dd(const void* p)  
{  
    const uint8_t* pb = (const uint8_t*)p;  
    pb += 7;  
    uint64_t r;  
    uint8_t* pd = (uint8_t*)&r;  
    for(int i=0; i<8; ++i) {  
        *pd++ = *pb--;  
    }  
    return r;  
} 