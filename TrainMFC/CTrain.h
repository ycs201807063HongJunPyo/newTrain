#pragma once
#include "afxdialogex.h"

#define RAIL_NUM 13	// 레일 개수

struct ThreadArg
{
	HWND hwnd;
	int type;
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
	afx_msg	void TimerFunction(int flag);
	afx_msg	void TrainMove(int left, int top, int right, int bottom);
	CRect trainSize;
	BOOL startFlag;
	


	BITMAP bmpInfo;  //비트맵
	CWinThread* m_thread_move[3];  //열차 쓰레드
	ThreadArg arg1;
	HwndArg hWndArg;

	int trainCount; //열차갯수

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg UINT DrawObject(LPVOID param);
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CStatic testText1;
	CStatic testText2;

};