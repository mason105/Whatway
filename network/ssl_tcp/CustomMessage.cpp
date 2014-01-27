#include "CustomMessage.h"


CustomMessage::CustomMessage()
{
	
}

void CustomMessage::SetMsgContent(std::string content)
{
	size_t MsgContentSize = content.size();

	if (MsgContentSize == 0)
		return;

	m_MsgContent.resize(MsgContentSize);

	//std::copy(content.begin(), content.end(), back_inserter(m_MsgContent));

	memcpy(m_MsgContent.data(), content.c_str(), MsgContentSize);
}

void CustomMessage::SetMsgHeader(unsigned char MsgType, int FunctionNo, unsigned char zip)
{
	memset(&m_MsgHeader, 0, sizeof(struct MsgHeader));

	//int nMsgContentSize = htonl(m_MsgContent.size());
	m_MsgHeader.MsgContentSize = m_MsgContent.size();

	m_MsgHeader.CRC = 0;
	m_MsgHeader.MsgType = MsgType;
	m_MsgHeader.FunctionNo = FunctionNo;
	m_MsgHeader.zip = zip;

}



bool CustomMessage::ParseMsgHeader()
{
	bool bRet = true;
	
	// java是网络字序，c++是主机字序，所以需要转换
	//int MsgContentSize = ntohl(m_MsgHeader.MsgContentSize);
	//m_MsgHeader.MsgContentSize = MsgContentSize;


	m_MsgContent.resize(m_MsgHeader.MsgContentSize);

	return bRet;
}

PMSG_HEADER CustomMessage::GetMsgHeader()
{
	return &m_MsgHeader;
}

char * CustomMessage::GetMsgContent()
{
	return m_MsgContent.data();
}

size_t CustomMessage::GetMsgContentSize()
{
	return m_MsgContent.size();
}

std::string CustomMessage::GetMsgContentString()
{
	std::string str(m_MsgContent.begin(), m_MsgContent.end());
	return str;

}

size_t CustomMessage::GetMsgHeaderSize()
{
	return sizeof(MSG_HEADER);
}

// 删除自己
void CustomMessage::destroy()
{
	boost::checked_delete(this);
}


void CustomMessage::SetRecvTime()
{
	boost::posix_time::ptime ptRecvTime =  boost::posix_time::microsec_clock::local_time();
	std::string recvTime = boost::gregorian::to_iso_extended_string(ptRecvTime.date()) + " " + boost::posix_time::to_simple_string(ptRecvTime.time_of_day());
	log.set_recvtime(recvTime);
}

void CustomMessage::SetSendTime()
{
	boost::posix_time::ptime ptSendTime = boost::posix_time::microsec_clock::local_time();
	std::string sSendTime = boost::gregorian::to_iso_extended_string(ptSendTime.date()) + " " + boost::posix_time::to_simple_string(ptSendTime.time_of_day());
	log.set_sendtime(sSendTime);
}



void CustomMessage::Log(Trade::TradeLog::LogLevel logLevel, std::string sysNo, std::string sysVer, std::string busiType, std::string funcId, std::string account, std::string clientIp, std::string request, std::string response, int status, std::string errCode, std::string errMsg, std::string beginTime, int runtime, std::string gatewayIp, std::string gatewayPort, std::string counterIp, std::string counterPort, std::string counterType)
{
	log.set_level(logLevel);

	log.set_sysno(sysNo);
	log.set_sysver(sysVer);	
	log.set_busitype(busiType);
	log.set_account(account);
	log.set_funcid(funcId);
	log.set_ip(clientIp);

	log.set_begintime(beginTime); 
	log.set_runtime(runtime);


	
	log.set_request(""); // flash
	log.set_request(request);
	log.set_status(status);
	log.set_retcode(errCode);
	log.set_retmsg(errMsg);
	log.set_response(response);
	log.set_enc_response(""); // flash
	
	log.set_gatewayip(gatewayIp);
	log.set_gatewayport(gatewayPort);

	log.set_gtip(counterIp);
	log.set_gtport(counterPort);
	log.set_countertype(counterType);
}
