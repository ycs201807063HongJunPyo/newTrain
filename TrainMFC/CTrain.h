#pragma once
#include "afxdialogex.h"

#define SUM_RAIL_NUM 5	// 선로 개수
#define TRAIN_HAVE_NUM 9 // 열차 개수

#define RAIL_NUM 13	// 레일 개수
#define LONG_RAIL_NUM 20	// 레일 개수
#define SUB_RAIL_NUM 7	// 레일 개수

#define PUBLIC_RAIL 4  // 4,5호선 공용 선로 구간

struct ThreadArg
{
	HWND hwnd;
	int type;  //어느 선로 사용할지
	UINT numberId;  // 스레드 번호
	BOOL isThreadWork;  // 작업중인 스레드인지 판단
	BOOL isTrainMove;  // 움직이는 열차인지 판단
	int moveCount;  // 열차 순환 횟수
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
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC); //배경
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); //글자 색, 글자 배경

	ThreadArg arg1[TRAIN_HAVE_NUM];  //쓰레드 인자

	CStatic hintText;
	CComboBox trainComboList;
	CEdit trainNumControll;
	afx_msg void OnEnSetfocusEditTrainnumber();
	CEdit controllTrain;
	afx_msg void OnCbnSelchangeComboTrainnumber();
};