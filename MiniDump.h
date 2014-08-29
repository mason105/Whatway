#pragma once
#include <Windows.h>
#include <DbgHelp.h>


// http://blog.csdn.net/fhxpp_27/article/details/9701867
// http://www.cnblogs.com/FCoding/archive/2012/07/05/2578543.html

class CMiniDump
{
private:
	LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter;

private:
	static LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pep);

public:
	BOOL BeginDump(VOID);
	BOOL EndDump(VOID);
	bool DontShowUI();
};

