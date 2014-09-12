#ifndef SYWG_CRC_DES_H
#define SYWG_CRC_DES_H

#include "stdafx.h"

typedef WORD    (__cdecl *PCalCRC)(void*, int);

typedef void    (__cdecl *PDedes)(char*);
typedef int     (__cdecl *PDesdone)();
typedef int     (__cdecl *PDesinit)(int);
typedef int     (__cdecl *PDessetkey)(char*);
typedef void    (__cdecl *PEndes)(char* );

class CSywg_CRC_DES
{
public:
	CSywg_CRC_DES(void);
	~CSywg_CRC_DES(void);

	HINSTANCE hinstLib; 

	PCalCRC             CalCrc;

	
	
	PDesinit            desinit;

	PDessetkey          dessetkey;

	PEndes              endes;
	PDedes              dedes;

	PDesdone            desdone;

	BOOL Load();
	void UnLoad();
};

extern CSywg_CRC_DES gSywg;

#endif
