#ifndef AGC_ENCRYPT_H
#define AGC_ENCRYPT_H

#include "stdafx.h"

WORD CalCRC(void *pData, int nDataLen);
void dedes(char *block);
int desdone();
int desinit(int mode);
int dessetkey(char *key);
void endes(char * block);
void UnixTimeStr(long t,char *buf);
long UnixTime();

#endif
