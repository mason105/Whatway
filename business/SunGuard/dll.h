#include <windows.h>
#ifndef DLL

#if defined(__cplusplus)
extern "C"{
#endif

__declspec( dllexport ) WORD CalCRC(void *pData, int nDataLen);
__declspec( dllexport ) void dedes(char *block);
__declspec( dllexport ) int desdone();
__declspec( dllexport ) int desinit(int mode);
__declspec( dllexport ) int dessetkey(char *key);
__declspec( dllexport ) void endes(char * block);
__declspec( dllexport )void UnixTimeStr(long t,char *buf);
__declspec( dllexport )long UnixTime();

#if defined(__cplusplus)
}
#endif 
#define DLL
#endif

