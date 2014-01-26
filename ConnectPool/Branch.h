#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

// stl
#include <map>
#include <vector>
#include <mutex>

// boost

#include "Counter.h"




class Branch
{
public:
	Branch(void);
	~Branch(void);
	
	int currentServerId;
	std::vector<Counter> servers;
	
	Counter* GetCounter();

	int GetServerCount();


};



#endif
