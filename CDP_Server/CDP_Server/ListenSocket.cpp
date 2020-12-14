#include "stdafx.h"
#include "ListenSocket.h"
#include "CDP_Server.h"
#include "CDP_ServerDlg.h"


CListenSocket::CListenSocket(CCDP_ServerDlg *pDlg)
{
	m_pDlg = pDlg;
}


CListenSocket::~CListenSocket()
{
}


void CListenSocket::OnAccept(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CSocket::OnAccept(nErrorCode);
	m_pDlg->ProcessAccept(nErrorCode);
}
