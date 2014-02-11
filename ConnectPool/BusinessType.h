#ifndef _BUSINESS_TYPE_
#define _BUSINESS_TYPE_

#include <string>
#include <map>

#include "common.h"
#include "Counter.h"
#include "ConnectPool.h"
#include "branch.h"

class BusinessType
{
public:
	BusinessType(void);
	~BusinessType(void);

	
	int counterType;

	std::map<std::string, ConnectPool*> connPool;

	std::map<std::string/*ÓªÒµ²¿id*/, Branch> branches;
};

#endif
