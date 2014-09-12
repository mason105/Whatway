#include <windows.h>
#include <time.h>

WORD CalCRC(void *pData, int nDataLen)
{
	WORD acc = 0;
	int i;
	unsigned char* Data = (unsigned char*) pData;

	while(nDataLen--)
	{
		acc = acc ^ (*Data++ << 8);
		for(i = 0; i++ < 8; )
			if(acc & 0x8000)
				acc = (acc << 1) ^ 0x1021;
			else
				acc <<= 1;
	}

	// Swap High and Low byte
	i = ( (WORD) ( ((UCHAR)acc) << 8) ) | ((WORD)(acc >> 8));

	return i;
}

long UnixTime()
{
	time_t t;

	t=time(NULL);
	return (long)t;
}

void UnixTimeStr(long t,char *buf)
{
	struct tm* h;
	time_t m;

	m=(time_t)t;
	h=localtime(&m);
	strftime(buf,80,"%Y%m%d",h);
}

