#include "IMessage.h"


IMessage::IMessage(void)
{
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