#ifndef _MYBOTAN_H_
#define _MYBOTAN_H_

#include <string>
#include <botan/botan.h>
//#include <botan/x931_rng.h>
//#include <botan/filters.h>
//#include <botan/lookup.h>

class CMyBotan
{
public:
	CMyBotan(void);
	~CMyBotan(void);

	void init(bool bThreadSafe=true);
	


	bool Base64Encoder(const unsigned char* src, int len, std::string& encoder);
	bool Base64Decoder(std::string& src, char* decoder, int * outlen);

	bool Base16Encoder(const unsigned char* src, int len, std::string& encoder);
	bool Base16Decoder(std::string& src, char* decoder, int * outlen);

	bool AESEncrypt(std::string algo, std::string key, std::string& src, char* cipher, int* outlen);
	bool AESDecrypt(std::string algo, std::string key, const unsigned char* src, int len, std::string& plain);

	bool AESEncrypt_Base64Encoder(std::string algo, std::string key, std::string& src, std::string& cipher);
	bool Base64Decoder_AESDecrypt(std::string algo, std::string key, std::string& cipher, std::string& plain);
	
	bool AESEncrypt_Base16Encoder(std::string algo, std::string key, std::string& src, std::string& cipher);
	bool Base16Decoder_AESDecrypt(std::string algo, std::string key, std::string& cipher, std::string& plain);

private:
	Botan::LibraryInitializer init_;
};
extern CMyBotan g_MyBotan;

#endif
