#pragma once
#include "afxsock.h"
class CCDP_ServerDlg;
class CDataSocket :
	public CSocket
{
public:
	CDataSocket(CCDP_ServerDlg *pDlg);
	CCDP_ServerDlg * m_pDlg;
	~CDataSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};

