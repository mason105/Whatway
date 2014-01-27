#include "MyBotan.h"


CMyBotan g_MyBotan;

CMyBotan::CMyBotan(void)
{
}

CMyBotan::~CMyBotan(void)
{
}

void CMyBotan::init(bool bThreadSafe)
{
	if (bThreadSafe)
	{
		std::string options = "thread_safe=true";
		init_.initialize(options);
	}
}

bool CMyBotan::Base64Encoder(const unsigned char* src, int len, std::string& encoder)
{
	try
	{
		if (src == NULL)
			return false;

		Botan::Pipe pipeBase64Enc(new Botan::Base64_Encoder());

		pipeBase64Enc.process_msg(src, len);

		

		encoder = pipeBase64Enc.read_all_as_string();
		if (encoder.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::Base64Decoder(std::string& src, char* decoder, int * outlen)
{
	try
	{
		if (src.empty())
			return false;

		if (decoder == NULL)
			return false;

		Botan::Pipe pipeBase64Dec(new Botan::Base64_Decoder());

		pipeBase64Dec.process_msg(src);
		
		Botan::SecureVector<unsigned char> sv;
		//Botan::MemoryVector<unsigned char> mv;

		sv = pipeBase64Dec.read_all(0);
		*outlen = sv.size();
		if (*outlen <= 0)
			return false;

		//std::string base64_decode(reinterpret_cast<const char*>(sv.begin()),sv.size());

		memcpy(decoder, sv.begin(), sv.size());

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::Base16Encoder(const unsigned char* src, int len, std::string& encoder)
{
	try
	{
		if (src == NULL)
			return false;

		Botan::Pipe pipeBase16Enc(new Botan::Hex_Encoder());

		pipeBase16Enc.process_msg(src, len);

		

		encoder = pipeBase16Enc.read_all_as_string();
		if (encoder.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::Base16Decoder(std::string& src, char* decoder, int * outlen)
{
	try
	{
		if (src.empty())
			return false;

		if (decoder == NULL)
			return false;

		Botan::Pipe pipeBase16Dec(new Botan::Hex_Decoder());

		pipeBase16Dec.process_msg(src);
		
		Botan::SecureVector<unsigned char> sv;
		//Botan::MemoryVector<unsigned char> mv;

		sv = pipeBase16Dec.read_all(0);
		*outlen = sv.size();
		if (*outlen <= 0)
			return false;

		//std::string base64_decode(reinterpret_cast<const char*>(sv.begin()),sv.size());

		memcpy(decoder, sv.begin(), sv.size());

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::AESEncrypt(std::string algo, std::string key, std::string& src, char* cipher, int* outlen)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;

		if (src.empty())
			return false;

		if (cipher == NULL)
			return false;

		//"AES-256/ECB/PKCS7"
		//AES-256/CBC/PKCS7
		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeAESEnc( Botan::get_cipher(algo, k, Botan::ENCRYPTION) );

		pipeAESEnc.process_msg(src);

		Botan::SecureVector<unsigned char> sv = pipeAESEnc.read_all();

		*outlen = sv.size();
		if (*outlen <= 0)
			return false;

		//char cipher[50];
		//memset(cipher, 0x00, sizeof(cipher));
		//int enc_len = sv_enc.size();
		memcpy(cipher, sv.begin(), sv.size());

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}


bool CMyBotan::AESDecrypt(std::string algo, std::string key, const unsigned char* src, int len, std::string& plain)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;


		if (src == NULL)
			return false;

		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeAESDec( Botan::get_cipher(algo, k, Botan::DECRYPTION) );

		pipeAESDec.process_msg(src, len);
		
		plain = pipeAESDec.read_all_as_string();
		if (plain.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::AESEncrypt_Base64Encoder(std::string algo, std::string key, std::string& src, std::string& cipher)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;

		if (src.empty())
			return false;

		//char aes_iv[16];
		//Botan::InitializationVector iv(aes_iv);

		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeAESEnc_B64Encoder( Botan::get_cipher(algo, k, Botan::ENCRYPTION), new Botan::Base64_Encoder );
		//Botan::Pipe pipeAESEnc( Botan::get_cipher("AES-256/ECB/PKCS7", key, iv, Botan::ENCRYPTION) );

		pipeAESEnc_B64Encoder.process_msg(src);

		cipher = pipeAESEnc_B64Encoder.read_all_as_string();
		if (cipher.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		
		return false;
	}
}


bool CMyBotan::Base64Decoder_AESDecrypt(std::string algo, std::string key, std::string& cipher, std::string& plain)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;

		if (cipher.empty())
			return false;

		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeB64Decoder_AESDec( new Botan::Base64_Decoder, Botan::get_cipher(algo, k, Botan::DECRYPTION) );
		//Botan::Pipe pipeAESDec( Botan::get_cipher("AES-256/ECB/PKCS7", key, iv, Botan::DECRYPTION) );

		pipeB64Decoder_AESDec.process_msg(cipher);

		plain = pipeB64Decoder_AESDec.read_all_as_string();
		if (plain.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool CMyBotan::AESEncrypt_Base16Encoder(std::string algo, std::string key, std::string& src, std::string& cipher)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;

		if (src.empty())
			return false;

		//char aes_iv[16];
		//Botan::InitializationVector iv(aes_iv);

		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeAESEnc_B16Encoder( Botan::get_cipher(algo, k, Botan::ENCRYPTION), new Botan::Hex_Encoder );
		//Botan::Pipe pipeAESEnc( Botan::get_cipher("AES-256/ECB/PKCS7", key, iv, Botan::ENCRYPTION) );

		pipeAESEnc_B16Encoder.process_msg(src);

		cipher = pipeAESEnc_B16Encoder.read_all_as_string();
		if (cipher.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}


bool CMyBotan::Base16Decoder_AESDecrypt(std::string algo, std::string key, std::string& cipher, std::string& plain)
{
	try
	{
		if (algo.empty())
			return false;

		if (key.empty())
			return false;

		if (cipher.empty())
			return false;

		Botan::SymmetricKey k((const unsigned char*)key.c_str(), key.length());

		Botan::Pipe pipeB16Decoder_AESDec( new Botan::Hex_Decoder, Botan::get_cipher(algo, k, Botan::DECRYPTION) );
		//Botan::Pipe pipeAESDec( Botan::get_cipher("AES-256/ECB/PKCS7", key, iv, Botan::DECRYPTION) );

		pipeB16Decoder_AESDec.process_msg(cipher);

		plain = pipeB16Decoder_AESDec.read_all_as_string();
		if (plain.empty())
			return false;

		return true;
	}
	catch (std::exception& e)
	{
		e.what();
		return false;
	}
	catch (...)
	{
		return false;
	}
}
