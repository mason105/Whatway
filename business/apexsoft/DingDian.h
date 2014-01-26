#ifndef _DINGDIAN_H_
#define _DINGDIAN_H_

#include <map>
#include <string>
#include <fstream>
#include <iostream>
/*
//连接句柄
#define HANDLE_CONN     long

//会话句柄
#define HANDLE_SESSION  long

typedef char* (*fpFix_Encode)(char * pData);
typedef long (*fpFix_GetCount)( HANDLE_SESSION sess );
typedef char* (*fpFix_GetErrMsg)( HANDLE_SESSION sess, char *out, int outlen );
typedef long (*fpFix_GetCode)( HANDLE_SESSION sess );
typedef BOOL (*fpFix_Run)( HANDLE_SESSION sess );
typedef BOOL (*fpFix_CreateHead)( HANDLE_SESSION sess, long nFunc );
typedef BOOL (*fpFix_SetGYDM)( HANDLE_SESSION sess, const char *val );
typedef BOOL (*fpFix_SetNode)( HANDLE_SESSION sess, const char *val );
typedef BOOL (*fpFix_SetDestFBDM)( HANDLE_SESSION sess, const char *val );
typedef BOOL (*fpFix_SetFBDM)( HANDLE_SESSION sess, const char *val );
typedef BOOL (*fpFix_SetWTFS)( HANDLE_SESSION sess, const char *val );
typedef BOOL (*fpFix_SetTimeOut)( HANDLE_SESSION sess, long timeout );
typedef BOOL (*fpFix_ReleaseSession)( HANDLE_SESSION sess );
typedef HANDLE_SESSION (*fpFix_AllocateSession)( HANDLE_CONN conn );
typedef BOOL (*fpFix_Close)( HANDLE_CONN conn );
typedef HANDLE_CONN  (*fpFix_Connect)( const char *pszAddr, const char *pszUser, const char *pszPwd, long nTimeOut );
typedef BOOL (*fpFix_Uninitialize)();
typedef BOOL (*fpFix_Initialize)();
typedef BOOL (*fpFix_SetItem)( HANDLE_SESSION sess, long id, const char *val );
typedef char* (*fpFix_GetItem)( HANDLE_SESSION sess, long id, char *out, int outlen, long row);
*/
class CDingDian
{
public:
	CDingDian(void);
	~CDingDian(void);

	BOOL m_bIsInited;

	// FID_KHH, 605
	std::map<std::string, long> m_mRequestField;

	// 功能号, [FID_CODE, 507]
	std::map<std::string, std::map<std::string, long>> m_mReturnField;

	void ReadMapFile();
	void ReadOutDomainFile();

	BOOL Init();
	void UnInit();

/*
	// 函数指针定义
	HMODULE hFixAPI;

	fpFix_Encode Fix_Encode;

	fpFix_GetCount Fix_GetCount;
	fpFix_GetErrMsg Fix_GetErrMsg;
	fpFix_GetCode Fix_GetCode;
	fpFix_Run Fix_Run;
	fpFix_CreateHead Fix_CreateHead;

	fpFix_SetGYDM Fix_SetGYDM;
	fpFix_SetNode Fix_SetNode;
	fpFix_SetDestFBDM Fix_SetDestFBDM;
	fpFix_SetFBDM Fix_SetFBDM;
	fpFix_SetWTFS Fix_SetWTFS;

	fpFix_SetTimeOut Fix_SetTimeOut;
	fpFix_ReleaseSession Fix_ReleaseSession;
	fpFix_AllocateSession Fix_AllocateSession;

	fpFix_Close Fix_Close;
	fpFix_Connect Fix_Connect;

	fpFix_Uninitialize Fix_Uninitialize;
	fpFix_Initialize Fix_Initialize;

	fpFix_SetItem Fix_SetItem;
	fpFix_GetItem Fix_GetItem;
*/
};
extern CDingDian g_DingDian;

#endif
