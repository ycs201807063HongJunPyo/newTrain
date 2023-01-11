#pragma once
#include "afxdialogex.h"

#define SUM_RAIL_NUM 5	// 선로 개수
#define TRAIN_HAVE_NUM 5 // 열차 개수

#define RAIL_NUM 13	// 레일 개수
#define LONG_RAIL_NUM 20	// 레일 개수
#define SUB_RAIL_NUM 6	// 레일 개수

struct ThreadArg
{
	HWND hwnd;
	int type;  //어느 선로 사용할지
	UINT numberId;  // 스레드 번호
};

struct HwndArg
{
	HWND hwnd;
};

// CTrain 대화 상자

class CTrain : public CDialog
{
	DECLARE_DYNAMIC(CTrain)

public:
	CTrain(CWnd* pParent = nullptr);   // 표준 생성자입니다.

	virtual ~CTrain();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINTRAIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCreate();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	CRect trainSize;

	CWinThread* m_thread_move[TRAIN_HAVE_NUM];  //열차 쓰레드
	ThreadArg arg1;
	HwndArg hWndArg;

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};