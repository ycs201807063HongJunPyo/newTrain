#pragma once
#include "afxdialogex.h"

#define THREAD_NUM 6	// 스레드 개수
#define RAIL_NUM 20		// 레일 개수
#define LINE_NUM 4		// 선로 개수

struct ThreadArg
{
	HWND hwnd;
	int type;				// 선로 번호
	UINT cycleCount;		// 순환 횟수
	BOOL checkCycleEnable;	// 순환 여부
	UINT id;				// 스레드 번호
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

	ThreadArg m_arg;
	HwndArg m_hWndArg;

	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};