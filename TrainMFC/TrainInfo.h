#pragma once
#include "afxdialogex.h"

#define TRAIN_INFO_NUM 9 // 열차 개수
struct ChildTrainInfo
{
	int ctiNumber;  //자식창 열차 정보_번호
	BOOL ctiWork;  //자식창 열차 정보_작업중인 스레드인지
	BOOL ctiMove;  //자식창 열차 정보_움직이는지
	int ctiCount;  //자식창 열차 정보_총 반복 횟수
	int ctiStation;  //자식창 열차 정보_위치
};


// TrainInfo 대화 상자

class TrainInfo : public CDialogEx
{
	DECLARE_DYNAMIC(TrainInfo)

public:
	TrainInfo(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~TrainInfo();
	afx_msg void SetFirstData(int number, int trainNumber, BOOL work, BOOL move, int count);
	afx_msg void SetUserData(int number, BOOL work, BOOL move);
	afx_msg void SetUserData(int number, int station);
	afx_msg void ChildDataDefault(int number);

	afx_msg void InvalidateChild(int number);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TrainInfo };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	CStatic* trainInfoStatic;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();

	ChildTrainInfo cti[TRAIN_INFO_NUM];  //자식창 열차 정보 배열
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
