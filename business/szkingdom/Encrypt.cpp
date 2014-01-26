#include "StdAfx.h"
#include ".\encrypt.h"




CEncrypt g_Encrypt;

CEncrypt::CEncrypt(void)
{
	hEncrypt = NULL;
	Encode = NULL;
}

CEncrypt::~CEncrypt(void)
{
	UnLoadDLL();
}

void CEncrypt::UnLoadDLL()
{
	if (hEncrypt != NULL)
	{
		FreeLibrary(hEncrypt);
		hEncrypt = NULL;
	}
}

bool CEncrypt::LoadDLL()
{
	TCHAR szPath[MAX_PATH];
	memset(szPath, 0x00, sizeof(szPath));
	GetModuleFileName(NULL, szPath, sizeof(szPath));

	CString sPath = szPath;


	int nPos = sPath.ReverseFind('\\');
	sPath = sPath.Mid(0, nPos);

	CString sDLLFileName = sPath + "\\dogskin.dll";
	

	hEncrypt = LoadLibrary(sDLLFileName);
	if (hEncrypt == NULL)
	{
		AfxMessageBox("load dogskin.dll error.");
		return false;
	}

	
	Encode = (fpEncode) GetProcAddress(hEncrypt, "Encode");
	if (Encode == NULL)
	{
		AfxMessageBox("GetProcAddress Encode error.");
		FreeLibrary(hEncrypt);
		return false;
	}

	return true;
}

std::string  CEncrypt::EncryptPWD(std::string sPassword)
{
	if (sPassword.empty())
		return "";

	

	try
	{
		char pMm[10]="",*pResult=NULL;
		std::string sEncryptPassword = "";

		pResult = Encode((char*)sPassword.c_str(), 0);
		if (pResult == NULL)
		{
			return "";
			
		}

		strcpy(pMm,pResult);
		sEncryptPassword = pMm;

		return sEncryptPassword;
	}
	catch (...)
	{
		return "";
	}

	
}
