#pragma once
#include "afxsock.h"
class CCDP_ServerDlg;
class CListenSocket :
	public CSocket
{
public:
	CListenSocket(CCDP_ServerDlg *pDlg);
	CCDP_ServerDlg *m_pDlg;
	~CListenSocket();
	virtual void OnAccept(int nErrorCode);
};

