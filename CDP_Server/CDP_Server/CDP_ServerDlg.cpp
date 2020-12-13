
// CDP_ServerDlg.cpp : 구현 파일
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
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

														// 구현입니다.
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


// CCDP_ServerDlg 대화 상자



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

			int 낙상 = frame.fsr1 + frame.fsr2 + frame.fsr3;

			//그림출력!
			// 낙상 출력
			if (낙상 >= 1 && frame.button == MID) //button(사이드레일 입력이 없고, fsr1,2,3의 합이 1이상일 경우
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1));
				pDlg->m_hazard.SetBitmap(hbit);
				for (int i = 0; i < 5; i++)//낙상일 경우 // 배경색 변경
				{
					pDlg->SetBackgroundColor(RGB(255, 0, 0));
					Sleep(500);
					pDlg->SetBackgroundColor(RGB(125, 75, 20));
					Sleep(50);
					pDlg->SetBackgroundColor(RGB(255, 255, 255));
				}
			}
			else if (낙상 < 1 || frame.button == HIGH)
			{
				pDlg->m_hazard.SetBitmap(NULL);
			}
			// ABS_FSR 출력
			if (frame.abs_fsr == LOW)
			{
				HBITMAP hbit;
				hbit = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP2));
				pDlg->m_abs.SetBitmap(hbit);
				for (int i = 0; i < 5; i++)//부재일 경우 // 배경색 변경
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
			//SOUND_SENSOR 출력
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
			//SIDE_RAIL 출력
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
			낙상 = 0;
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
	while (pArg->Thread_run) //스레드가 실행되는 동안
	{

		pArg->date->tm_isdst = 1;// 센서 제어 정보 // 1 : 센서 동작  // 0 : 센서 동작 정지
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
// CCDP_ServerDlg 메시지 처리기
BOOL CCDP_ServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(RGB(255, 255, 255)); // 배경색 설정
	SetWindowTheme(GetDlgItem(IDC_STATIC_GROUP_BOX)->m_hWnd, _T(""), _T("")); // IDC 색상 설정

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

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

									// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
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
			AfxMessageBox(_T("서버를 시작합니다."), MB_ICONINFORMATION);
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
	
	AfxMessageBox(_T("이미 실행 중인 서버가 있습니다.")_T("\n프로그램을 종료합니다."), MB_ICONERROR);
	return FALSE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCDP_ServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);


	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
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
	//		str.Format(_T("client IP 주소 : %s, 포트 번호 : %d!\n"), PeerAddr, PeerPort);
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
	message.Format(_T("### [%s, %d] 접속 종료 ###"), recvAddr, recvPort);
	m_rx_edit.SetSel(len, len);
	m_rx_edit.ReplaceSel(message);
}


void CCDP_ServerDlg::OnBnClickedSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CString tx_message;
	//m_tx_edit_short.GetWindowText(tx_message);
	//tx_message += _T("\r\n");
	//tx_cs.Lock();
	//arg1.pList->AddTail(tx_message); //arg1.pList에 tx_message를 끝에 추가한다.
	//tx_cs.Unlock();
	//m_tx_edit_short.SetWindowText(_T("")); //m_tx_edit_short 비우기
	//m_tx_edit_short.SetFocus();

	//int len = m_tx_edit.GetWindowTextLengthW();
	//m_tx_edit.SetSel(len, len);
	//m_tx_edit.ReplaceSel(tx_message); //보낸 tx_message 띄우기

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
	//if (TX_run == 1) BUTTON = 0; // 주석 해제시 토글 모드 on!!!! 삭제하지 말 것
}


void CCDP_ServerDlg::OnStnClickedPic1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
