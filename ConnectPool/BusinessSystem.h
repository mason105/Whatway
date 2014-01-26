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


	std::map<BUSI_TYPE, BusinessType > busis;

};

#endif