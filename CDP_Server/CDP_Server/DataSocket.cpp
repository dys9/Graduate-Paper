#include "stdafx.h"
#include "DataSocket.h"
#include "CDP_ServerDlg.h"
#include "CDP_Server.h"

CDataSocket::CDataSocket(CCDP_ServerDlg *pDlg)
{
	m_pDlg = pDlg;
}


CDataSocket::~CDataSocket()
{
}


void CDataSocket::OnReceive(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CSocket::OnReceive(nErrorCode);
	m_pDlg->ProcessReceive(this, nErrorCode);
}


void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CSocket::OnClose(nErrorCode);
	m_pDlg->ProcessClose(this, nErrorCode);
}
