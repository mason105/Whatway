#include "stdafx.h"

#include "MiniDump.h"

#include <string>
#include "config\ConfigManager.h"

#pragma comment(lib, "Dbghelp.lib")


LONG WINAPI CMiniDump::MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pep)  
{  
	std::string file = "crash.dmp";
	
	HANDLE hFile = CreateFile(file.c_str(), FILE_ALL_ACCESS, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	
	MINIDUMP_EXCEPTION_INFORMATION mdei;
	mdei.ThreadId			= GetCurrentThreadId();
	mdei.ExceptionPointers = pep;
	mdei.ClientPointers	= TRUE;

	//MINIDUMP_CALLBACK_INFORMATION mci;  
    //mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
    //mci.CallbackParam       = 0; 
	
	BOOL Success = ::MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,				
		MiniDumpNormal,
		(pep != 0) ? &mdei : 0,	
		NULL,
		NULL);



	CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
	
}


BOOL CMiniDump::BeginDump(VOID)
{
	PreviousExceptionFilter = SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	return TRUE;
}

BOOL CMiniDump::EndDump(VOID)
{
	SetUnhandledExceptionFilter(PreviousExceptionFilter);

	return TRUE;
}


bool CMiniDump::DontShowUI()
{
	HKEY hRoot = HKEY_CURRENT_USER;
	HKEY hWER;
	HKEY hLocalDump;
	HKEY hTradeGateway;
	LSTATUS result;

	

	result = RegOpenKeyEx(hRoot, TEXT("SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting"), 0, KEY_ALL_ACCESS, &hWER);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("打开注册表失败.");
		return false;

	}

	BYTE val = 1;

	result = RegSetValueEx(hWER, TEXT("DontShowUI"), 0, REG_DWORD, &val, 4);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("设置DontShowUI失败.");
		return false;
	}
	/*
	result = RegOpenKeyEx(hWER, TEXT("LocalDumps"), 0, KEY_ALL_ACCESS, &hLocalDump);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("打开注册表失败.");
		return false;

	}
	*/
	result = RegCreateKey(hWER, TEXT("LocalDumps"), &hLocalDump);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("创建子键LocalDumps失败.");
		return false;

	}

	result = RegCreateKey(hLocalDump, TEXT("TradeGateway.exe"), &hTradeGateway);
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("创建TradeGateway.exe子键失败.");
		return false;

	}

	std::string path = gConfigManager::instance().m_sPath;

	result = RegSetValueEx(hTradeGateway, TEXT("DumpFolder"), 0, REG_SZ, (const BYTE*)path.c_str(), path.length());
	if (result != ERROR_SUCCESS)
	{
		AfxMessageBox("写DumpFolder失败.");
		return false;

	}

	
	RegCloseKey(hWER);

	return true;
}
/*
BOOL PreventSetUnhandledExceptionFilter()
{
          HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
         if (hKernel32 ==   NULL)
             return FALSE;
 
 
         void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
         if(pOrgEntry == NULL)
             return FALSE;
 
 
         unsigned char newJump[ 100 ];
         DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
         dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
 
 
         void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
         DWORD dwNewEntryAddr = (DWORD) pNewFunc;
         DWORD dwRelativeAddr = dwNewEntryAddr -  dwOrgEntryAddr;
 
 
         newJump[ 0 ] = 0xE9;  // JMP absolute
         memcpy(&newJump[ 1 ], &dwRelativeAddr, sizeof(pNewFunc));
         SIZE_T bytesWritten;
         BOOL bRet = WriteProcessMemory(GetCurrentProcess(),    pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
         return bRet;
}
*/