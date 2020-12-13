
// CDP_ServerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "CDP_Server.h"
#include "CDP_ServerDlg.h"
#include "afxdialogex.h"
#include "ListenSocket.h"
#include "DataSocket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define MAX_CLIENT 10
#define HIGH 1
#define MID 0
#define LOW -1
CFile file;
CString state;
int BUTTON = 0; int TX_run = 0;
CCriticalSection tx_cs;
CCriticalSection rx_cs;
CDataSocket *temp_client[MAX_CLIENT] = { 0 };
CWinThread* _RXTHD[MAX_CLIENT] = { 0 };
int idx_m = 0;
// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

														// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//	ON_WM_CTLCOLOR()
//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCDP_ServerDlg ��ȭ ����



CCDP_ServerDlg::CCDP_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CDP_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCDP_ServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, rx_bed);
	DDX_Control(pDX, IDC_EDIT4, m_rx_edit);
	//DDX_Control(pDX, IDC_EDIT5, m_tx_edit_short);
	DDX_Control(pDX, IDC_EDIT6, m_tx_edit);
	DDX_Control(pDX, IDC_PIC1, m_hazard);
	DDX_Control(pDX, IDC_PIC2, m_abs);
	DDX_Control(pDX, IDC_PIC3, m_ns);
	DDX_Control(pDX, IDC_PIC4, m_button);
}

BEGIN_MESSAGE_MAP(CCDP_ServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(SEND, &CCDP_ServerDlg::OnBnClickedSend)
	ON_STN_CLICKED(IDC_PIC1, &CCDP_ServerDlg::OnStnClickedPic1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//RXTHREAD
UINT RXThread(LPVOID arg)
{
	ThreadArg_Receive *pArg = (ThreadArg_Receive *)arg;
	CList<Frame> *plist = pArg->pDataList;
	CCDP_ServerDlg *pDlg = (CCDP_ServerDlg *)pArg->pDlg;
	CStringList * stringlist = NULL;
	stringlist = new CStringList;
	while (pArg->Thread_run)
	{
		//AfxMessageBox(_T("while (pArg->Thread_run)"));
		POSITION pos = plist->GetHeadPosition();
		POSITION current_pos;

		time_t timer;
		timer = time(NULL);
		while (pos != NULL)
		{
			//file.Open(_T("FILE.DAT"), CFile::modeCreate | CFile::modeReadWrite);

			struct tm *t = localtime(&timer);
			CString message, temp;
			CString time_message;

			current_pos = pos;
			rx_cs.Lock();
			Frame frame = plist->GetNext(pos);
			rx_cs.Unlock();

			time_message.Format(_T("[%d/%2d/%2d/%2d/%2d]"), t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
			file.Write(time_message, time_message.GetLength() * sizeof(TCHAR));

			message.Append(_T("Bed_No : "));
			temp.Format(_T("%3d "), frame.bed_no);
			message.Append(temp);
			pDlg->rx_bed.SetWindowTextW(temp);
			//message.Append(temp+(CString)",  ABS_FSR: ");

			file.Write(message, message.GetLength() * sizeof(TCHAR));

			message = " ABS_FSR: ";
			temp.Format(_T("%d"), frame.abs_fsr);
			message.Append(temp + (CString)",FSR1: ");
			temp.Format(_T("%d"), frame.fsr1);
			message.Append(temp + (CString)",FSR2: ");
			temp.Format(_T("%d"), frame.fsr2);
			message.Append(temp + (CString)",FSR3: ");
			temp.Format(_T("%d"), frame.fsr3);
			message.Append(temp + (CString)",Noise: ");
			temp.Format(_T("%d"), frame.noise);
			message.Append(temp + (CString)",SideRail: ");
			temp.Format(_T("%d"), frame.button);
			message.Append(temp + (CString)"\n");
			//pDlg->m_rx_edit.SetWindowText(message);
			stringlist->AddTail(message);
			
			file.Write(message, message.GetLength() * sizeof(TCHAR));
			
			//file.Close();

			POSITION mpos = stringlist->GetHeadPosition();
			POSITION mcurrent_pos;
			//AfxMessageBox(message);


			pDlg->m_rx_edit.GetWindowText(message);////////////////
			while (mpos != NULL)
			{
				mcurrent_pos = mpos;
				pDlg->m_rx_edit.SetWindowText(stringlist->GetNext(mpos));
			}

			//pDlg->m_rx_edit.SetWindowText(message);//////////////////
			//pDlg->m_rx_edit.LineScroll(pDlg->m_rx_edit.GetLineCount());

			int ���� = frame.fsr1 + frame.fsr2 + frame.fsr3;

			//�׸����!
			// ���� ���
			if (���� >= 1 && frame.button == MID) //button(���̵巹�� �Է��� ����, fsr1,2,3�� ���� 1�̻��� ���
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1));
				pDlg->m_hazard.SetBitmap(hbit);
				for (int i = 0; i < 5; i++)//������ ��� // ���� ����
				{
					pDlg->SetBackgroundColor(RGB(255, 0, 0));
					Sleep(500);
					pDlg->SetBackgroundColor(RGB(125, 75, 20));
					Sleep(50);
					pDlg->SetBackgroundColor(RGB(255, 255, 255));
				}
			}
			else if (���� < 1 || frame.button == HIGH)
			{
				pDlg->m_hazard.SetBitmap(NULL);
			}
			// ABS_FSR ���
			if (frame.abs_fsr == LOW)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP2));
				pDlg->m_abs.SetBitmap(hbit);
				for (int i = 0; i < 5; i++)//������ ��� // ���� ����
				{
					pDlg->SetBackgroundColor(RGB(255, 255, 20));
					Sleep(100);
					pDlg->SetBackgroundColor(RGB(255, 255, 255));
				}
			}
			else if (frame.abs_fsr != LOW)
			{
				pDlg->m_abs.SetBitmap(NULL);
			}
			//SOUND_SENSOR ���
			if (frame.noise == HIGH)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP3));
				pDlg->m_ns.SetBitmap(hbit);
			}
			else if (frame.noise == MID)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP5));
				pDlg->m_ns.SetBitmap(hbit);
			}
			else if (frame.noise == LOW)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP6));
				pDlg->m_ns.SetBitmap(hbit);
			}
			//SIDE_RAIL ���
			if (frame.button == HIGH)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP7));
				pDlg->m_button.SetBitmap(hbit);
			}
			else if (frame.button == MID)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP4));
				pDlg->m_button.SetBitmap(hbit);
			}

			plist->RemoveAt(current_pos);
			���� = 0;
		}
		Sleep(10);
	}
	//file.Close();
	return 0;
}
//TXTHREAD
UINT TXThread(LPVOID arg)//Transmit (Tx)
{
	ThreadArg_Send *pArg = (ThreadArg_Send*)arg;
	CCDP_ServerDlg *pDlg = (CCDP_ServerDlg *)pArg->pDlg;

	time_t timer;
	timer = time(NULL);
	pArg->date = localtime(&timer);
	while (pArg->Thread_run) //�����尡 ����Ǵ� ����
	{

		pArg->date->tm_isdst = 1;// ���� ���� ���� // 1 : ���� ����  // 0 : ���� ���� ����
		CString ymd, temp;
		ymd.Format(_T("%d"), pArg->date->tm_year + 1900);
		temp.Format(_T("%d"), pArg->date->tm_mon + 1);
		ymd.Append((CString)"/" + temp);
		temp.Format(_T("%d"), pArg->date->tm_mday);
		ymd.Append((CString)"/" + temp);
		temp.Format(_T("%d"), pArg->date->tm_hour);
		ymd.Append((CString)"/   " + temp);
		temp.Format(_T("%d"), pArg->date->tm_min);
		ymd.Append((CString)":" + temp);
		temp.Format(_T("%d"), pArg->date->tm_sec);
		ymd.Append((CString)":" + temp + (CString)"\n");
		//AfxMessageBox(ymd);
		
		// IN CRITICAL SECTION
		tx_cs.Lock();
		if (BUTTON == 1)
		{
			TX_run = 1;
			//AfxMessageBox(ymd);
			pDlg->m_tx_edit.SetWindowText(ymd);
			pArg->date->tm_mon += 1;
			//pDlg->m_pDataSocket->Send(pArg->date, sizeof(tm));
			for (int i = 0; i < MAX_CLIENT; i++)
			{
				if ( temp_client[i] != NULL)
					temp_client[i]->Send(pArg->date, sizeof(tm));
			}
			Sleep(10000);
		}
		timer = time(NULL);
		pArg->date = localtime(&timer);
		tx_cs.Unlock();
	}return 0;
}
// CCDP_ServerDlg �޽��� ó����
BOOL CCDP_ServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(RGB(255, 255, 255)); // ���� ����
	SetWindowTheme(GetDlgItem(IDC_STATIC_GROUP_BOX)->m_hWnd, _T(""), _T("")); // IDC ���� ����

	static CFont  font;
	//font = new CFont();
	LOGFONT LogFont;

	GetDlgItem(IDC_STATIC)->GetFont()->GetLogFont(&LogFont);

	LogFont.lfWeight = 1000;
	LogFont.lfHeight = 20;

	font.CreateFontIndirect(&LogFont);
	GetDlgItem(IDC_STATIC_FALL)->SetFont(&font);
	GetDlgItem(IDC_STATIC_ABS)->SetFont(&font);
	GetDlgItem(IDC_STATIC_NOISE)->SetFont(&font);
	GetDlgItem(IDC_STATIC_SIDERAIL)->SetFont(&font);

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

									// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	
									// TX Queue
	arg1.Thread_run = 1;
	arg1.pDlg = this;

	// RX Queue
	//CList<Frame>* newlist2 = new CList<Frame>;
	//arg2.pDataList = newlist2;
	//arg2.Thread_run = 1;
	//arg2.pDlg = this;
	CList<Frame>* newlist2 = new CList<Frame>[MAX_CLIENT];
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		arg2[i].Thread_run = 1;
		arg2[i].idx = 0;
		arg2[i].pDlg = this;
		arg2[i].pDataList = &newlist2[i];
	}

	WSADATA wsa;
	int error_code;
	if ((error_code = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0)
	{
		TCHAR buffer[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL);
		AfxMessageBox(buffer, MB_ICONERROR);
	}

	m_pListenSocket = NULL;
	m_pDataSocket = NULL;

	file.Open(_T("FILE.DAT"), CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone);


	//ASSERT(m_pListenSocket == NULL);
	m_pListenSocket = new CListenSocket(this);
	if (m_pListenSocket->Create(50000))
	{
		if (m_pListenSocket->Listen())
		{
			AfxMessageBox(_T("������ �����մϴ�."), MB_ICONINFORMATION);
			pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1);
			//pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2);
			return TRUE;
		}
	}
	else
	{
		int err = m_pListenSocket->GetLastError();
		TCHAR buffer[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL);
		AfxMessageBox(buffer, MB_ICONERROR);
	}
	
	AfxMessageBox(_T("�̹� ���� ���� ������ �ֽ��ϴ�.")_T("\n���α׷��� �����մϴ�."), MB_ICONERROR);
	return FALSE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CCDP_ServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CCDP_ServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);


	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CCDP_ServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCDP_ServerDlg::ProcessAccept(int nErrorCode)
{
	CString PeerAddr;
	UINT PeerPort;
	CString str;
	ASSERT(nErrorCode == 0);

	//if (m_pDataSocket == NULL)
	//{
	//	m_pDataSocket = new CDataSocket(this);
	//	if (m_pListenSocket->Accept(*m_pDataSocket))
	//	{
	//		m_pDataSocket->GetPeerName(PeerAddr, PeerPort);
	//		str.Format(_T("client IP �ּ� : %s, ��Ʈ ��ȣ : %d!\n"), PeerAddr, PeerPort);
	//		m_rx_edit.SetWindowTextW(str);
	//	}
	//	else
	//	{
	//		delete m_pDataSocket;
	//		m_pDataSocket = NULL;
	//	}
	//}
	while (temp_client[idx_m] == NULL)
	{
		temp_client[idx_m] = new CDataSocket(this);
		if (m_pListenSocket->Accept(*temp_client[idx_m]))
		{
			temp_client[idx_m]->GetPeerName(PeerAddr, PeerPort);
			str.Format(_T("[IDX = %d] Client IP : %s, Port : %d\n"), idx_m+1,PeerAddr, PeerPort);
			arg2[idx_m].port = PeerPort;
			_RXTHD[idx_m] = AfxBeginThread(RXThread, (LPVOID)&arg2[idx_m]);
			arg2[idx_m].idx = idx_m;
			m_rx_edit.SetWindowTextW(str);
			AfxMessageBox(str, MB_ICONINFORMATION);
			str.Format(_T("idx  = %d"), idx_m);

			idx_m++;
			break;
		}
	}
}


void CCDP_ServerDlg::ProcessReceive(CDataSocket* pSocket, int nErrorCode)
{
	char pBuf[1024 + 1];
	CString strData;
	Frame data;
	int nbytes;


	nbytes = pSocket->Receive(&data, sizeof(Frame));
	pBuf[nbytes] = NULL;
	//strData = (LPCTSTR)pBuf;;
	//CString temp;
	//temp = pBuf;
	//AfxMessageBox(temp);

	CString message = (CString)"Bed_No: ", temp;
	temp.Format(_T("%d"), data.bed_no);
	//message.Append(temp+(CString)",  ABS_FSR: ");
	message = "ABS_FSR: ";
	temp.Format(_T("%d"), data.abs_fsr);
	message.Append(temp + (CString)", FSR1: ");
	temp.Format(_T("%d"), data.fsr1);
	message.Append(temp + (CString)", FSR2: ");
	temp.Format(_T("%d"), data.fsr2);
	message.Append(temp + (CString)", FSR3: ");
	temp.Format(_T("%d"), data.fsr3);
	message.Append(temp + (CString)", Noise: ");
	temp.Format(_T("%d"), data.noise);
	message.Append(temp + (CString)", SideRail: ");
	temp.Format(_T("%d"), data.button);
	message.Append(temp + (CString)"\n");
	AfxMessageBox(message);

	UINT recvPort;
	CString recvAddr;


	int idx_n = 0;
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		rx_cs.Lock();
		temp_client[i]->GetPeerName(recvAddr, recvPort);
		rx_cs.Unlock();
		if ( recvPort == arg2[i].port )
		{
			idx_n = i;
			break;
		}
	}

	rx_cs.Lock();
	arg2[idx_n].pDataList->AddTail(data);
	rx_cs.Unlock();
}


void CCDP_ServerDlg::ProcessClose(CDataSocket* pSocket, int nErrorCode)
{

	UINT recvPort;
	CString recvAddr;
	int idx_n = 0;
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		rx_cs.Lock();
		temp_client[i]->GetPeerName(recvAddr, recvPort);
		rx_cs.Unlock();
		if (recvPort == arg2[i].port)
		{
			idx_n = i;
			break;
		}
	}
	pSocket->Close();
	delete m_pDataSocket;
	m_pDataSocket = NULL;
	int len = m_rx_edit.GetWindowTextLengthW();
	m_rx_edit.SetWindowText(NULL);
	CString message;
	message.Format(_T("### [%s, %d] ���� ���� ###"), recvAddr, recvPort);
	m_rx_edit.SetSel(len, len);
	m_rx_edit.ReplaceSel(message);
}


void CCDP_ServerDlg::OnBnClickedSend()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	//CString tx_message;
	//m_tx_edit_short.GetWindowText(tx_message);
	//tx_message += _T("\r\n");
	//tx_cs.Lock();
	//arg1.pList->AddTail(tx_message); //arg1.pList�� tx_message�� ���� �߰��Ѵ�.
	//tx_cs.Unlock();
	//m_tx_edit_short.SetWindowText(_T("")); //m_tx_edit_short ����
	//m_tx_edit_short.SetFocus();

	//int len = m_tx_edit.GetWindowTextLengthW();
	//m_tx_edit.SetSel(len, len);
	//m_tx_edit.ReplaceSel(tx_message); //���� tx_message ����

	//struct tm *Date;
	//time_t timer;

	//timer = time(NULL);
	//Date = localtime(&timer);
	//char a[] = "hello";
	//tx_cs.Lock();
	//arg1.date = Date;
	////m_pDataSocket->Send(a, sizeof(a));
	//m_pDataSocket->Send(Date, sizeof(tm));
	//tx_cs.Unlock();

	//CString ymd, temp;
	//ymd.Format(_T("%d"), Date->tm_year+1900);
	//temp.Format(_T("%d"), Date->tm_mon);
	//ymd.Append((CString)"/" + temp);
	//temp.Format(_T("%d"), Date->tm_mday);
	//ymd.Append((CString)"/"+temp);
	//temp.Format(_T("%d"), Date->tm_hour);
	//ymd.Append((CString)"/" + temp);
	//temp.Format(_T("%d"), Date->tm_min);
	//ymd.Append((CString)"/" + temp);
	//AfxMessageBox(ymd);
	BUTTON = 1;
	//if (TX_run == 1) BUTTON = 0; // �ּ� ������ ��� ��� on!!!! �������� �� ��
}


void CCDP_ServerDlg::OnStnClickedPic1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	HBITMAP hbit;

	hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(1000));

	m_hazard.SetBitmap(hbit);
}


HBRUSH CCDP_ServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor); 
	switch (nCtlColor) 
	{ case IDC_STATIC_GROUP_BOX: //CTLCOLOR_STATIC
		pDC->SetTextColor(RGB(255, 0, 0)); 
		pDC->SetBkColor(RGB(255, 0, 0));
		return (HBRUSH)GetStockObject(NULL_BRUSH); 
		break; 
	}

}
