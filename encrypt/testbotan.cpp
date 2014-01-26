#include <string>
#include <iostream>
#include "MyBotan.h"

int botan_test()
{
	try
	{
		
		g_MyBotan.init();

		bool bRet;

		char src[20];
		memset(src, 0x00, sizeof(src));
		strcpy(src, "990818");
		int len = strlen(src);

		std::string encoder;
		g_MyBotan.Base64Encoder((const unsigned char*)src, len, encoder);

		
		char decoder[20];
		memset(decoder, 0x00, sizeof(decoder));
		int outlen;
		g_MyBotan.Base64Decoder(encoder, decoder, &outlen);

		std::string algo = "AES-256/ECB/PKCS7";
		std::string key = "23dpasd23d-23l;df9302hzv/3lvjh*5";
		std::string plain = "990818";
		char cipher[20];
		memset(cipher, 0x00, sizeof(cipher));
		bRet = g_MyBotan.AESEncrypt(algo, key, plain, cipher, &outlen);
		
		std::string plaintext;
		bRet = g_MyBotan.AESDecrypt(algo, key, (const unsigned char*)cipher, outlen, plaintext);

		std::string ciphertext;
		g_MyBotan.AESEncrypt_Base64Encoder(algo, key, plaintext, ciphertext);

		std::string plaintext2;
		g_MyBotan.Base64Decoder_AESDecrypt(algo, key, ciphertext, plaintext2);
		
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
