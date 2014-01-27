#ifndef __ORA_CONN_POOL_H
#define __ORA_CONN_POOL_H

#ifdef WIN32
	#define OCI_API __stdcall 
	#define OCI_CHARSET_ANSI
	//#define OCI_CHARSET_WIDE
	//#define OCI_CHARSET_MIXED
#else
#endif

//#include <afxmt.h>
#include <string>
#include "ocilib.h"
#include <boost/thread/detail/singleton.hpp>



class OracleConnPool
{
/*
public:
	static OracleConnPool* Instance();
protected:
	OracleConnPool(void);
private:
	static OracleConnPool* _instance;
*/

public:
	OracleConnPool(void);
	~OracleConnPool(void);
	bool CreateConnPool(std::string sUserName, std::string sPassword, std::string sServiceName, int nMin, int nMax, int nIncrease);
	void CloseConnPool();

	OCI_Connection* GetConnect();
	void FreeConnect(OCI_Connection * conn);

	void ExecutePLSQL();

private:
	OCI_ConnPool* m_pool;

	bool m_bInit;
	std::string m_sServiceName;
	std::string m_sUserName;
	std::string m_sPassword;
	int m_nMin;
	int m_nMax;
	int m_nIncrease;



};


typedef boost::detail::thread::singleton<OracleConnPool> DBConnPool;

#endif

