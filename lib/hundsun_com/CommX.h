// CommX.h: interface for the CCommX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMX_H__024DACEC_C603_4BA2_B93E_183F368CB2AF__INCLUDED_)
#define AFX_COMMX_H__024DACEC_C603_4BA2_B93E_183F368CB2AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include ".\debug\hscommx.tlh"

class CCommX  
{
public:
	CCommX();
	virtual ~CCommX();

	bool init();


};

extern CCommX g_CommX;

#endif // !defined(AFX_COMMX_H__024DACEC_C603_4BA2_B93E_183F368CB2AF__INCLUDED_)
