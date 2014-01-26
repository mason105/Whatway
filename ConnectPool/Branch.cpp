#include "Branch.h"


Branch::Branch(void)
{
	currentServerId = 0;
}


Branch::~Branch(void)
{
}

Counter* Branch::GetCounter()
{
	currentServerId++;

	if (currentServerId >= servers.size())
		currentServerId = 0;

	Counter& counter = servers[currentServerId];

	return &counter;
}

int Branch::GetServerCount()
{
	return servers.size();
}
