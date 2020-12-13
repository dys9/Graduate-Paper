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
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CSocket::OnReceive(nErrorCode);
	m_pDlg->ProcessReceive(this, nErrorCode);
}


void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CSocket::OnClose(nErrorCode);
	m_pDlg->ProcessClose(this, nErrorCode);
}
