#include "IMessage.h"
#include <boost/checked_delete.hpp>


IMessage::IMessage()
{
	//m_MsgHeader.resize(msgHeaderSize);
	//this->session = session;
}


IMessage::~IMessage(void)
{
}

ISession* IMessage::GetSession()
{
	return session;
}
void IMessage::SetSession(ISession* sess)
{
	session = sess;
}

std::vector<char>& IMessage::GetMsgHeader()
{
	return m_MsgHeader;
}

size_t IMessage::GetMsgHeaderSize()
{
	return m_MsgHeader.size();
}

std::vector<char>& IMessage::GetMsgContent()
{
	return m_MsgContent;
}

size_t IMessage::GetMsgContentSize()
{
	return m_MsgContent.size();
}

void IMessage::destroy()
{
	boost::checked_delete(this);
}

void IMessage::SetMsgContent(std::string content)
{
	size_t MsgContentSize = content.size();

	if (MsgContentSize == 0)
		return;

	m_MsgContent.resize(MsgContentSize);

	//std::copy(content.begin(), content.end(), back_inserter(m_MsgContent));

	memcpy(m_MsgContent.data(), content.c_str(), MsgContentSize);
}

void IMessage::Log(Trade::TradeLog::LogLevel logLevel, std::string sysNo, std::string sysVer, std::string busiType, std::string funcId, std::string account, std::string clientIp, std::string request, std::string response, int status, std::string errCode, std::string errMsg, std::string beginTime, int runtime, std::string gatewayIp, std::string gatewayPort, std::string counterIp, std::string counterPort, std::string counterType)
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

void IMessage::SetRecvTime()
{
	boost::posix_time::ptime ptRecvTime =  boost::posix_time::microsec_clock::local_time();
	std::string recvTime = boost::gregorian::to_iso_extended_string(ptRecvTime.date()) + " " + boost::posix_time::to_simple_string(ptRecvTime.time_of_day());
	log.set_recvtime(recvTime);
}

void IMessage::SetSendTime()
{
	boost::posix_time::ptime ptSendTime = boost::posix_time::microsec_clock::local_time();
	std::string sSendTime = boost::gregorian::to_iso_extended_string(ptSendTime.date()) + " " + boost::posix_time::to_simple_string(ptSendTime.time_of_day());
	log.set_sendtime(sSendTime);
}


std::string IMessage::GetMsgContentString()
{
	std::string str(m_MsgContent.begin(), m_MsgContent.end());
	return str;

}

void IMessage::SetMsgHeader(std::vector<char> header)
{
	m_MsgHeader = header;
}
