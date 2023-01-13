#pragma once
#include "afxdialogex.h"

#define SUM_RAIL_NUM 5	// 선로 개수
#define TRAIN_HAVE_NUM 9 // 열차 개수

#define RAIL_NUM 13	// 레일 개수
#define LONG_RAIL_NUM 20	// 레일 개수
#define SUB_RAIL_NUM 7	// 레일 개수

struct ThreadArg
{
	HWND hwnd;
	int type;  //어느 선로 사용할지
	UINT numberId;  // 스레드 번호
	BOOL isThreadWork;  // 작업중인 스레드인지 판단
	BOOL isTrainMove;  // 움직이는 열차인지 판단
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
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCreate();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();

	CWinThread* m_thread_move[TRAIN_HAVE_NUM];  //열차 쓰레드
	ThreadArg arg1[TRAIN_HAVE_NUM];

	afx_msg void OnPaint();
	//배경
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//글자 색, 글자 배경
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CStatic hintText;
	CComboBox trainComboList;
};