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
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CSocket::OnAccept(nErrorCode);
	m_pDlg->ProcessAccept(nErrorCode);
}
