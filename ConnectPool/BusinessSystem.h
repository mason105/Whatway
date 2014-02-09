#ifndef _BUSINESS_SYTEM_H
#define _BUSINESS_SYTEM_H

#include <string>
#include <map>
#include <vector>

#include "Counter.h"
#include "BusinessType.h"

class BusinessSystem
{
public:
	BusinessSystem(void);
	~BusinessSystem(void);

	std::string id;
	std::string desc;


	std::map<int/*业务类型*/, BusinessType > busis;

};

#endif