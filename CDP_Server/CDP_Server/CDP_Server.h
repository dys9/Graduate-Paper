
// CDP_Server.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CCDP_ServerApp:
// �� Ŭ������ ������ ���ؼ��� CDP_Server.cpp�� �����Ͻʽÿ�.
//

class CCDP_ServerApp : public CWinApp
{
public:
	CCDP_ServerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CCDP_ServerApp theApp;