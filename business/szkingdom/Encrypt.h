
#pragma once

#include <string>

typedef char * (*fpEncode) (char *, int); 

class CEncrypt
{
public:
	CEncrypt(void);
	~CEncrypt(void);

	fpEncode Encode;
	HMODULE hEncrypt;

	bool LoadDLL();
	void UnLoadDLL();

	std::string EncryptPWD(std::string sPassword);
	
};

extern CEncrypt g_Encrypt;
