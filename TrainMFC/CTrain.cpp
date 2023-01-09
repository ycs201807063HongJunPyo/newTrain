﻿// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

#define TRAIN_TIMER 10
#define WAIT_TIMER 11
// CTrain 대화 상자
IMPLEMENT_DYNAMIC(CTrain, CDialog)

//열차 구역 내선
int railTopLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };
int railTopRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };
int railBottomLeft[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };
int railBottomRight[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };
//열차 구역 1회
int soloRailTopLeft[SMALL_RAIL_NUM] = { 860, 960, 1060, 1160, 1160,1160,1160,1260, 1360 };
int soloRailTopRight[SMALL_RAIL_NUM] = { 960, 1060, 1160, 1260, 1260,1260,1260,1360, 1460 };
int soloRailBottomLeft[SMALL_RAIL_NUM] = { 210, 210, 210, 210, 260, 310,360,360, 360 };
int soloRailBottomRight[SMALL_RAIL_NUM] = {  260, 260, 260, 260 , 310, 360,410,410, 410};
//열차 구역 반복(one train NEED)
int subRailTopLeft[SUB_RAIL_NUM] = { 760, 760, 760 };
int subRailTopRight[SUB_RAIL_NUM] = { 810, 810, 810 };
int subRailBottomLeft[SUB_RAIL_NUM] = { 310, 210, 110 };
int subRailBottomRight[SUB_RAIL_NUM] = { 410, 310, 210 };


BOOL insCheck[3][RAIL_NUM] = { FALSE, };
BOOL oneTrainInsCheck = FALSE;  //열차 구역 반복(one train NEED) Check
BOOL startInsCheck[3] = { FALSE, }; //시작값 체크

//열차 기본값
int trainAreaFlag;
int trainCount; //열차갯수


CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	startFlag = TRUE;
	trainTimerWait = TRUE;
	trainCount = 0;
}

CTrain::~CTrain()
{
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}


BEGIN_MESSAGE_MAP(CTrain, CDialog)
	ON_BN_CLICKED(IDCREATE, &CTrain::OnBnClickedCreate)
	ON_BN_CLICKED(IDSTART, &CTrain::OnBnClickedStart)
	ON_BN_CLICKED(IDSTOP, &CTrain::OnBnClickedStop)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()

END_MESSAGE_MAP()


// CTrain 메시지 처리기
UINT ThreadMoveTrain(LPVOID param);

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	arg1.hwnd = this->m_hWnd;
	trainAreaFlag = GetDlgItemInt(IDC_EDIT_LINE);
	if (trainAreaFlag >= 4 || trainAreaFlag == 0) {
		trainAreaFlag = 1;
	}
	//oneTrain Check
	if (oneTrainInsCheck == TRUE && trainAreaFlag==3) {
		OutputDebugStringW(_T("create\n"));
		trainAreaFlag = 0;
		trainCount--;
	}
	arg1.type = trainAreaFlag;
	if (trainCount <= 4 && trainTimerWait == TRUE && startInsCheck[(trainAreaFlag-1)] == FALSE ) {
		SetTimer(TRAIN_TIMER, 500, NULL);
		trainTimerWait = FALSE;
		m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &arg1, THREAD_PRIORITY_NORMAL, 0, 0);
		trainCount++;
	}
}

void CTrain::OnBnClickedStart()
{
	for (int i = 0; i < trainCount; i++)
	{
		m_thread_move[i]->ResumeThread();
	}

	//SetTimer(TRAIN_TIMER, 50, NULL);
}


void CTrain::OnBnClickedStop()
{
	for (int i = 0; i < trainCount; i++)
	{
		m_thread_move[i]->SuspendThread();
	}
}

void CTrain::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	//역 테두리 설정
	CPen myPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = dc.SelectObject(&myPen);
	int arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(railTopLeft[i], railBottomLeft[i], railTopRight[i], railBottomRight[i]);
	}
	arraySize = (sizeof(soloRailTopLeft) / sizeof(*soloRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(soloRailTopLeft[i], soloRailBottomLeft[i], soloRailTopRight[i], soloRailBottomRight[i]);
	}
	arraySize = (sizeof(subRailTopLeft) / sizeof(*subRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(subRailTopLeft[i], subRailBottomLeft[i], subRailTopRight[i], subRailBottomRight[i]);
	}
	dc.SelectObject(oldPen);

}

BOOL CTrain::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rect;
	GetClientRect(rect);
	pDC->FillSolidRect(rect, RGB(0, 0, 0));
	return CDialog::OnEraseBkgnd(pDC);
}

UINT DrawObject(LPVOID param, int type)
{
	CTrain* pMain = (CTrain*)param;
	HwndArg* pArg = (HwndArg*)param;

	//동적할당 알아보고 그걸로 바꿀수있으면 바꿔주기(지금은 무조건 13인데 이러면 작은 선로는 낭비됨)
	int threadRailTopLeft[RAIL_NUM] = { 0 };
	int threadRailTopRight[RAIL_NUM] = { 0 };
	int threadRailBottomLeft[RAIL_NUM] = { 0 };
	int threadRailBottomRight[RAIL_NUM] = { 0 };

	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;  //열차 렉트
	int trainSpeed = 1;  //열차 속도

	int trainX;  //x기억
	int trainY;  //y기억

	int trainWidth = 30;	//열차 가로 길이
	int trainHeight = 30;	//열차 세로 길이

	int flag=0;			//열차 방향

	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치

	BOOL lineWhile = TRUE;  //반복구간 플래그용 변수
	int arraySize;

	int lineSelect;
	lineSelect = type-1;

	int stationCount = 0; //현재 도착역
	int subStationCount = 0; //정차이후 출발역
	int safeStationCount = 1; //안전거리

	int lineRangeX;  //trainGoToSizeX
	int lineRangeY;  //trainGoToSizeY


	if (lineSelect == 0) {
		flag = 1;
		trainSpeed = 0;  //열차 속도
		posX = 0;
		posY = 20;
		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = railTopLeft[i];
			threadRailTopRight[i] = railTopRight[i];
			threadRailBottomLeft[i] = railBottomLeft[i];
			threadRailBottomRight[i] = railBottomRight[i];
		}
	}
	else if (lineSelect == 1) {
		flag = 1;
		trainSpeed = 870;  //열차 속도
		arraySize = (sizeof(soloRailBottomLeft) / sizeof(*soloRailBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = soloRailTopLeft[i];
			threadRailTopRight[i] = soloRailTopRight[i];
			threadRailBottomLeft[i] = soloRailBottomLeft[i];
			threadRailBottomRight[i] = soloRailBottomRight[i];
		}
		posX = 870;
		posY = 220;
		trainX = posX + 30;
	}
	else if (lineSelect == 2) {
		oneTrainInsCheck = TRUE;
		flag = 4;
		trainSpeed = 0;  //열차 속도
		arraySize = (sizeof(subRailBottomLeft) / sizeof(*subRailBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = subRailTopLeft[i];
			threadRailTopRight[i] = subRailTopRight[i];
			threadRailBottomLeft[i] = subRailBottomLeft[i];
			threadRailBottomRight[i] = subRailBottomRight[i];
		}
		posX = 810;
		trainX = posX;
		posY = 380;
	}


	CRect MainRectTest;
	CRect tmpRect;

	dc.Attach(hdc);

	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역

	while (1) {
		Sleep(10);
		//열차 속도 설정

		if (insCheck[lineSelect][safeStationCount] == FALSE || lineSelect == 2) {
			//오른(1), 아래(2) 플래그
			if (flag <= 2) {
				trainSpeed += 10;
			}
			//왼(3), 위(4) 플래그
			else if (flag <= 4) {
				trainSpeed -= 10;
			}

		}
		//열차 움직임 조정
			//열차 이동

		if (flag == 1) {
			//오른
			rect = CRect(trainSpeed, posY, trainSpeed + trainWidth, posY + trainHeight);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == 2) {
			//T : 일반용 아래 / F : 반복 구간용 아래
			lineSelect != 2 ? rect = CRect(trainX - trainWidth, trainSpeed, trainX, trainHeight + trainSpeed) : rect = CRect(posX - trainWidth, trainSpeed + posY, posX, posY + trainSpeed + trainHeight);
			trainY = trainSpeed + trainHeight;
		}
		else if (flag == 3) {
			//왼
			rect = CRect(trainSpeed, trainY - trainHeight, trainSpeed + trainWidth, trainY);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == 4) {
			//위
			rect = CRect(trainX - trainWidth, posY + trainSpeed, trainX, posY + trainHeight + trainSpeed);
			trainY = trainSpeed + trainHeight;
		}

		InvalidateRect(pArg->hwnd, rect, TRUE);
		///역 색칠하기///
		CBrush brush;
		CBrush* oldBrush = dc.SelectObject(&brush);

		dc.SelectObject(oldBrush);
		dc.Rectangle(rect);	//열차 그리기

		stationRect = CRect(threadRailTopLeft[stationCount], threadRailBottomLeft[stationCount], threadRailTopRight[stationCount], threadRailBottomRight[stationCount]);
		subStationRect = CRect(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]);
		//반복구간이 아니고 겹쳐있다면
		if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0 && lineSelect != 2)
		{
			insCheck[lineSelect][stationCount] = TRUE;
		}

		if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0)
		{
			//색칠 + 무효화
			brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
			oldBrush = dc.SelectObject(&brush);
			dc.Rectangle(stationRect);
			//이전 부분과 충돌이 있을경우에만 무효화 해주기
			if (IntersectRect(tmpRect, rect, subStationRect) && !insCheck[lineSelect][subStationCount] && (stationCount >= 1 || lineWhile == FALSE)) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			}
		}
		else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
		{
			//반복 구간 아닐때
			if (lineSelect != 2) {
				insCheck[lineSelect][subStationCount] = FALSE;
			}
			//색만 칠하기
			brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
			oldBrush = dc.SelectObject(&brush);
			dc.Rectangle(subStationRect);
		}
		//정차, 방향 조정하기
		lineRangeX = (threadRailTopRight[stationCount] - threadRailTopLeft[stationCount]) / 2;
		lineRangeY = (threadRailBottomRight[stationCount] - threadRailBottomLeft[stationCount]) / 2;
		//2호선 내선, 외선 연결
		if (lineSelect == 0) {
			// 위, 아래 이동
			if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)
			{
				flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T : 하, F : 상
				if ((trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
				{
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					safeStationCount = stationCount + 1;

					if (safeStationCount == 13) {
						safeStationCount = 0;
					}
					if (stationCount == 7) {
						//왼쪽
						flag = 3;
						trainSpeed = trainX - trainWidth;
					}

					else if (stationCount == 13) {
						//오른쪽(초기화용)
						startInsCheck[lineSelect] = TRUE;
						insCheck[lineSelect][subStationCount] = FALSE;
						flag = 1;
						stationCount = 0;
						safeStationCount = 1;
						trainSpeed = 50;
						trainX = 0;
						trainY = 0;
						posX = 0;
						posY = 20;
					}

				}
			}
			// 좌, 우 이동
			else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)
			{
				flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T : 좌, F : 우
				if ((lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
				{
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					lineSelect == 0 ? safeStationCount = stationCount + 1 : safeStationCount++;

					if (lineSelect == 0) {
						if (stationCount == 5) {
							//아래쪽
							flag = 2;
							trainSpeed = 20;

						}
						else if (stationCount == 11) {
							//새로 생기는거 안전 구역
							startInsCheck[lineSelect] = TRUE;
							flag = 4;
							trainSpeed = trainY - trainHeight;
							posY = 0;
						}
					}

				}
			}
			// 생성 후 이동
			else if (stationCount == 0)
			{
				if ((1 == flag || 3 == flag) && (lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
				{
					startInsCheck[lineSelect] = TRUE;
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					lineSelect == 0 ? safeStationCount = stationCount + 1 : safeStationCount++;
				}
			}
		}
		//1회
		else if (lineSelect == 1) {
			// 위, 아래 이동
			if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)
			{
				flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T : 하, F : 상

				if ((trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
				{
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					safeStationCount = stationCount + 1;
					if (stationCount == 7) {
						//right
						flag = 1;
						posY = rect.top;
						trainSpeed = rect.left;
					}
				}
			}
			// 좌, 우 이동
			else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)
			{
				flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T : 좌, F : 우
				if ((lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
				{
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					lineSelect == 0 ? safeStationCount = stationCount + 1 : safeStationCount++;
					if (stationCount == 4) {
						//아래쪽
						flag = 2;
						trainSpeed = stationRect.top;
					}
					else if (stationCount == 9) {
						//End
						trainCount--;
						insCheck[lineSelect][subStationCount] = FALSE;
						InvalidateRect(pArg->hwnd, stationRect, TRUE);
						UpdateWindow(pArg->hwnd);
						return 0;
					}
				}
			}
			// 생성 후 이동
			else if (stationCount == 0)
			{
				if ((1 == flag || 3 == flag) && (lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
				{
					startInsCheck[lineSelect] = TRUE;
					subStationCount = stationCount;
					Sleep(500);
					stationCount++;
					lineSelect == 0 ? safeStationCount = stationCount + 1 : safeStationCount++;
				}
			}
		}
		else if (lineSelect == 2) {

			if ((trainSpeed + posY + (flag * 20)) == threadRailBottomRight[stationCount]-10) {
				subStationCount = stationCount;
				Sleep(500);
				lineWhile == TRUE ? stationCount++ : stationCount--;
				if (stationCount == 3) {
					//아래쪽
					posX = 790;
					flag = 2;
					lineWhile = FALSE;
					subStationCount = 2;
					stationCount = 1;
				}
				else if (stationCount == -1) {
					//위쪽
					posX = 810;
					flag = 4;
					lineWhile = TRUE;
					subStationCount = 0;
					stationCount = 1;
				}
			}
		}
		//나는 처음에있고, 다른열차가 2번에 있는데 열차 생성하면 겹쳐나오는 오류있음
		if (stationCount == 2 && insCheck[lineSelect][0] == FALSE && insCheck[lineSelect][11] == FALSE && insCheck[lineSelect][12] == FALSE) {
			startInsCheck[lineSelect] = FALSE;
		}
	}
	return 1;
}

UINT ThreadMoveTrain(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	ThreadArg* pArg = (ThreadArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush = RGB(255, 255, 255);
	DWORD dwResult;
	int errorCode = 0;

	dc.Attach(hdc);
	dc.SelectObject(&brush);

	switch (pArg->type)
	{
	case 1:
		errorCode = DrawObject(pMain, pArg->type);
		break;
	case 2:
		errorCode = DrawObject(pMain, pArg->type);
		break;
	case 3:
		errorCode = DrawObject(pMain, pArg->type);
		break;
	default:
		errorCode = 100;
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
		break;
	}

	if (errorCode >= 5000)
	{
		::GetExitCodeThread(pMain->m_thread_move[(errorCode - 5000)], &dwResult);
		trainCount--;
	}

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);

	return 0;
}

void CTrain::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	switch (nIDEvent)
	{
	case TRAIN_TIMER:
		//열차 반복제어용 타이머
		trainTimerWait = TRUE;
		break;

		CDialog::OnTimer(nIDEvent);
	}
}

/*
CString test;
			test.Format(L"%d", bmpInfo.bmHeight + trainSpeed);
			subStationCountext1.SetWindowTextW(test);
			test.Format(L"%d", trainCount);
			subStationCountext2.SetWindowTextW(test);
*/


HBRUSH CTrain::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	if (nCtlColor == CTLCOLOR_STATIC)	//STATIC 컨트롤만 배경색 변경
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 변경
	hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;
}
