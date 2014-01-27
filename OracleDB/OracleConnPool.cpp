#include "oracleconnpool.h"
#include <iostream>
//#include "log.h"


/*
OracleConnPool* OracleConnPool::_instance = 0;

OracleConnPool* OracleConnPool::Instance()
{
	if (_instance == 0)
	{
		_instance = new OracleConnPool();
	}

	return _instance;
}
*/
OracleConnPool::OracleConnPool(void)
{
	m_pool = NULL;
}

OracleConnPool::~OracleConnPool(void)
{
}


bool OracleConnPool::CreateConnPool(std::string sUserName, std::string sPassword, std::string sServiceName,  int nMin, int nMax, int nIncrease)
{
	m_sServiceName = sServiceName;
	m_sUserName = sUserName;
	m_sPassword = sPassword;
	m_nMin = nMin;
	m_nMax = nMax;
	m_nIncrease = nIncrease;

	if (!OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
	{
		OCI_Error *err = OCI_GetLastError();
		int nErrCode = OCI_ErrorGetOCICode(err);
		const char* sErrMsg = OCI_ErrorGetString(err);
		std::cout << "OCI_Initialize errcode = " << nErrCode << ", errmsg = " << sErrMsg << "\n";

		return false;

	}

	m_pool = OCI_PoolCreate(m_sServiceName.c_str(), m_sUserName.c_str(), m_sPassword.c_str(), OCI_POOL_CONNECTION, OCI_SESSION_DEFAULT, m_nMin, m_nMax, m_nIncrease);
	//m_pool = OCI_PoolCreate("stockpooldb", "stockpool", "stockpool", OCI_POOL_CONNECTION, OCI_SESSION_DEFAULT, m_nMin, m_nMax, m_nIncrease);
	if (m_pool == NULL)
	{
		m_pool = NULL;

		OCI_Error *err = OCI_GetLastError();
		int nErrCode = OCI_ErrorGetOCICode(err);
		const char* sErrMsg = OCI_ErrorGetString(err);
		std::cout << "OCI_PoolCreate errcode = " << nErrCode << ", errmsg = " << sErrMsg << "\n";

		return false;
	}

	//OCI_PoolSetTimeout(m_pool, 15); //10;

	return true;
}

void OracleConnPool::CloseConnPool()
{
	if (m_pool != NULL)
	{
		OCI_PoolFree(m_pool);

		OCI_Cleanup();

		m_pool = NULL;


	}
}


OCI_Connection* OracleConnPool::GetConnect()
{
	if (m_pool == NULL)
	{
		return NULL;
	}

	OCI_Connection *cn = NULL;
	cn = OCI_PoolGetConnection(m_pool, NULL);

	if (cn == NULL)
	{
	}
	else
	{
	}

	return cn;
}

void OracleConnPool::FreeConnect(OCI_Connection * conn)
{
	if (conn == NULL)
		return;
	
	OCI_ConnectionFree(conn);	
}

/*
void OracleConnPool::ExecutePLSQL()
{
		OCI_Connection *cn = NULL;
//		cn = g_OracleConnPool.GetConnect();
		if (cn == NULL)
			return;
		
		OCI_Statement *st;
		st = OCI_StatementCreate(cn);






		int result = 0;
		
		std::string sql = "begin\n TRANSFER_TRADE_DATA();\n end;";

		result = OCI_ExecuteStmt(st, sql.c_str());

		OCI_Error *err = OCI_GetLastError();
        //TRACE("errcode %d, errmsg %s", OCI_ErrorGetOCICode(err),                                         OCI_ErrorGetString(err));


		OCI_Commit(cn);

		OCI_ConnectionFree(cn);	
}
*/
