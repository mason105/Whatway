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

	
	COUNTER_TYPE counterType;

	std::map<std::string, ConnectPool*> connPool;

	std::map<std::string, Branch*> branches;
};

#endif
