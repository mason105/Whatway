#pragma once
#include <string>
#include <map>

/*
8:30清掉所有数据，初始化
有数据，就返回
没有数据，从柜台读取数据，返回，并保存
*/
class CCacheData
{
public:
	CCacheData(void);
	~CCacheData(void);

	// key=hash(request), value=response
	std::map<std::string, std::string> m_mapCacheData;



	void Start();
	void Stop();
	void* m_hThread; // HANDLE
	static void Run(void* params);

	bool m_bRunning;
};

extern CCacheData g_CacheData;