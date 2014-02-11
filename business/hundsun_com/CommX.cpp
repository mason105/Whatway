// CommX.cpp: implementation of the CCommX class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CommX.h"
#include "ConfigManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace HsCommX;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommX g_CommX;

CCommX::CCommX()
{

}

CCommX::~CCommX()
{

}

bool CCommX::init()
{int ret;

	CString addr = "168.168.111.22";
	CString key = "";
	//CoInitialize(NULL);

   CLSID clsid;
   CLSIDFromProgID(OLESTR("HsCommX.Comm"),&clsid);

   CComPtr<IComm> pComm;//智能指针
   pComm.CoCreateInstance(clsid);
   pComm->Create();
   ret = pComm->ConnectX(1, _bstr_t("168.168.111.22"), 9001, 0, _bstr_t(""), 0);
   
   //pGetRes->Hello();
   //pGetRes.Release();//小心哦!!请看最后的“注意”
   //CoUninitialize();
//	comm.CreateDispatch("HsCommX.Comm");
//	comm.Create();
	


	
	//= comm.ConnectX(1, addr, 9001, 0, key, 0);
	if (ret == 0)
	{
		AfxMessageBox("连接成功\n");
		return true;
	}
	else
	{
		AfxMessageBox("连接失败\n");
		return false;
	}
	
}
