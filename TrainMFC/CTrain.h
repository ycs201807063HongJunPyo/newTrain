#pragma once
#include "afxdialogex.h"

#define THREAD_NUM 3	// 스레드 개수
#define RAIL_NUM 17		// 레일 개수
#define LINE_NUM 10		// 선로 개수

struct ThreadArg
{
	HWND hwnd;
	int type;
	UINT id;
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


	BITMAP bmpInfo;  //비트맵
	CWinThread* m_thread_move[THREAD_NUM];  //열차 쓰레드
	ThreadArg arg1;
	HwndArg hWndArg;

	int trainCount; //열차갯수

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CStatic testText1;
	CStatic testText2;

	afx_msg void OnEnChangeEditLine();
};