
// CDP_ServerDlg.h : 헤더 파일
//

#pragma once
#pragma commnet(lib, "UXTheme.lib")
#include "afxwin.h"
#include "afxcoll.h"
#include "DataSocket.h"
#include "ListenSocket.h"
#include <time.h>
#define MAX_CLIENT 10
typedef struct Frame
{
	int bed_no = 0;
	int abs_fsr = 0;
	int fsr1 = 0;
	int fsr2 = 0;
	int fsr3 = 0;
	int noise = 0;
	int button = 0;
	int sock = 0;
};
struct ThreadArg_Send
{
	struct tm *date;
	CDialogEx *pDlg;
	int Thread_run;
	int idx;
};
struct ThreadArg_Receive
{
	CDialogEx *pDlg;
	CList<Frame>* pDataList;
	int Thread_run;
	int idx;
	UINT port;
};
class CListenSocket;
class CDataSocket;
// CCDP_ServerDlg 대화 상자
class CCDP_ServerDlg : public CDialogEx
{
	// 생성입니다.
public:
	CCDP_ServerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

											// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CDP_SERVER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//CButton m_ButtonBox;
	//m_ButtonBox.Create(_T("Time"), WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_GROUP, CRect(50, 050, 200, 200), this, 1111);
	CEdit rx_bed;
	//	CEdit rx_abs;
	//	CEdit rx_ns;
	ThreadArg_Send arg1;
	ThreadArg_Receive arg2[MAX_CLIENT];
	CWinThread *pThread1, *pThread2;
	CListenSocket *m_pListenSocket;
	CDataSocket *m_pDataSocket = NULL;
	void ProcessAccept(int nErrorCode);
	CEdit m_rx_edit;
	void ProcessReceive(CDataSocket* pSocket, int nErrorCode);
	void ProcessClose(CDataSocket* pSocket, int nErrorCode);
	afx_msg void OnBnClickedSend();
	CEdit m_tx_edit_short;
	CEdit m_tx_edit;
	CStatic m_hazard;
	afx_msg void OnStnClickedPic1();
	CStatic m_abs;
	CStatic m_ns;
	CStatic m_button;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
