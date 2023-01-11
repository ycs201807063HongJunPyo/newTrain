﻿// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

// CTrain 대화 상자

IMPLEMENT_DYNAMIC(CTrain, CDialog)

//열차 구역
int firstRailLeft[RAIL_NUM] = { 10, 110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400 };

int secondRailLeft[RAIL_NUM] = { 10, 10, 110, 110, 110, 10, 10, 10, 110, 220, 220, 220, 220, 220, 220, 120 };
int secondRailTop[RAIL_NUM] = { 170, 220, 220, 270, 320, 320, 370, 420, 420, 420, 380, 320, 270, 210, 160, 160 };
int secondRailRight[RAIL_NUM] = { 110, 110, 210, 210, 210, 110, 110, 110, 220, 310, 310, 310, 310, 310, 310, 220 };
int secondRailBottom[RAIL_NUM] = { 220, 270, 270, 320, 370, 370, 420, 470, 470, 470, 420, 380, 320, 270, 210, 210 };

int thirdRailLeft[RAIL_NUM] = { 310, 420, 620, 920 };
int thirdRailTop[RAIL_NUM] = { 210, 210, 210, 210 };
int thirdRailRight[RAIL_NUM] = { 420, 620, 920, 1420 };
int thirdRailBottom[RAIL_NUM] = { 270, 270, 270, 270 };

BOOL insCheck[LINE_NUM][RAIL_NUM] = { FALSE }; //역에 열차가 있는지 확인
BOOL startInsCheck[LINE_NUM] = { FALSE };	// 시작역에 열차가 있는지 확인

//겹치는 구역
BOOL rectResult;
CRect testRect;

CString lineEditText = _T("");	//이전 선로 번호
int trainCount;					//열차갯수


CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	trainCount = 0;
	arg1 = { NULL };
	hWndArg = { NULL };
	*m_thread_move = { NULL };
}

CTrain::~CTrain()
{
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	GetDlgItem(IDC_CheckCir)->SetWindowTextW(_T("순환 가능"));
}


BEGIN_MESSAGE_MAP(CTrain, CDialog)
	ON_BN_CLICKED(IDCREATE, &CTrain::OnBnClickedCreate)
	ON_BN_CLICKED(IDSTART, &CTrain::OnBnClickedStart)
	ON_BN_CLICKED(IDSTOP, &CTrain::OnBnClickedStop)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//ON_WM_CTLCOLOR()

END_MESSAGE_MAP()


// CTrain 메시지 처리기
UINT ThreadMoveTrain(LPVOID param);

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString tmpStr = _T("");

	GetDlgItemText(IDC_EDIT_LINE, tmpStr);
	UINT numLine = _wtoi(tmpStr);			//선로 번호
	GetDlgItemText(IDC_EDITCirCount, tmpStr);
	UINT numCirCount = "" == tmpStr || "0" == tmpStr ? 1 : _wtoi(tmpStr);	//순환 횟수
	BOOL checkCirEnable = ((CButton*)GetDlgItem(IDC_CheckCir))->GetCheck();	//순환 여부
	
	arg1.hwnd = this->m_hWnd;
	arg1.type = numLine;
	arg1.cycleCount = numCirCount;
	arg1.checkCycleEnable = checkCirEnable;
	if (trainCount <= (THREAD_NUM - 1)) {
		arg1.id = trainCount + 1000;
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
	CDC MemDC;

	MemDC.CreateCompatibleDC(&dc);	// 화면 DC와 호환되는 메모리 DC를 생성

	//역 테두리 설정
	CPen myPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = dc.SelectObject(&myPen);
	int arraySize = (sizeof(firstRailLeft) / sizeof(*firstRailLeft)) / 2;

	//역 만들기
	for (int i = 0; i < arraySize; i++)
	{
		dc.Rectangle(CRect(firstRailLeft[i], 10, firstRailLeft[i + 1], 30));
		dc.Rectangle(CRect(firstRailLeft[i], 40, firstRailLeft[i + 1], 60));
	}
	for (int i = 0; i < RAIL_NUM; i++) {
		dc.Rectangle(secondRailLeft[i], secondRailTop[i], secondRailRight[i], secondRailBottom[i]);
		dc.Rectangle(thirdRailLeft[i], thirdRailTop[i], thirdRailRight[i], thirdRailBottom[i]);
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

void DrawFillRect(LPVOID param, CRect fillRect, int r, int g, int b)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush;
	CBrush* oldBrush;

	dc.Attach(hdc);

	//현재 영역 칠하기
	brush.CreateSolidBrush(RGB(r, g, b));
	oldBrush = dc.SelectObject(&brush);
	dc.Rectangle(fillRect);
	dc.SelectObject(oldBrush);
	brush.DeleteObject();

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

UINT DrawObject(LPVOID param, int type, UINT cycleCount, BOOL checkCycleEnable, UINT id)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect train;			//열차
	int trainSpeed = 0;		//열차 속도
	int trainX = 50;		//x기억
	int trainY = 50;		//y기억
	int trainWidth = 50;	//열차 가로 길이
	int trainHeight = 25;	//열차 세로 길이
	int flag = 1;			//열차 이동 방향		1 : Right / 2 : Down / 3 : Left / 4 : Up
	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재역 좌표 번호
	int subStationCount = 0;//이전역 좌표 번호
	int lineSelect = 0;		//선로 번호(저장용)

	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역

	CRect tmpRect;			//곂치는 영역 저장
	BOOL moveEnable = TRUE;	//이동 가능 여부
	BOOL changeDirection[3] = { TRUE, TRUE, FALSE };	//1,2 : 방향 전환 여부(좌,우 <-> 상,하), 3 : 정방향, 역방향 여부(좌표 변경용)
	BOOL isReverse = FALSE;	//정방향, 역방향 여부(방향 전환용)
	
	// 선로 좌표 저장
	int TrailLeft[RAIL_NUM] = { 0 };
	int TrailTop[RAIL_NUM] = { 0 };
	int TrailRight[RAIL_NUM] = { 0 };
	int TrailBottom[RAIL_NUM] = { 0 };

	// 임시 선로 좌표 저장
	int tmpTrailLeft[RAIL_NUM] = { 0 };
	int tmpTrailTop[RAIL_NUM] = { 0 };
	int tmpTrailRight[RAIL_NUM] = { 0 };
	int tmpTrailBottom[RAIL_NUM] = { 0 };

	CString testStr;		///Test String
	int count = 0;			//실제 선로 개수
	int cirCount = 0;		//순환 횟수(증가용)
	int cirCountText = 1;	//순환 횟수(저장용)
	BOOL cycleEnable = FALSE;	//순환 가능 여부
	BOOL enableInvalid = FALSE;	//화면 업데이트 여부
	int arraySize = 0;
	

	cirCountText = (cycleCount * 2);
	cycleEnable = checkCycleEnable;

	switch (type)	//선로 선택
	{
	case 1:
		arraySize = (sizeof(firstRailLeft) / sizeof(*firstRailLeft)) / 2;
		for (int i = 0; i < arraySize; i++)
		{
			TrailLeft[i] = firstRailLeft[i];
			TrailTop[i] = 10;
			TrailRight[i] = firstRailLeft[i + 1];
			TrailBottom[i] = 30;
			count = i;
		}
		trainY = 30;
		trainHeight = 20;
		break;
	case 2:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = secondRailLeft[i];
			TrailTop[i] = secondRailTop[i];
			TrailRight[i] = secondRailRight[i];
			TrailBottom[i] = secondRailBottom[i];

			if (TrailLeft[i] == 0 && TrailTop[i] == 0 && TrailRight[i] == 0 && TrailBottom[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posY = 160;
		break;
	case 3:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = thirdRailLeft[i];
			TrailTop[i] = thirdRailTop[i];
			TrailRight[i] = thirdRailRight[i];
			TrailBottom[i] = thirdRailBottom[i];

			if (TrailLeft[i] == 0 && TrailTop[i] == 0 && TrailRight[i] == 0 && TrailBottom[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posX = 300;
		posY = 210;
		break;
	case 4:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = secondRailLeft[i];
			TrailTop[i] = secondRailTop[i];
			TrailRight[i] = secondRailRight[i];
			TrailBottom[i] = secondRailBottom[i];

			if (i > 13)
			{
				TrailLeft[i] = thirdRailLeft[i - 14];
				TrailTop[i] = thirdRailTop[i - 14];
				TrailRight[i] = thirdRailRight[i - 14];
				TrailBottom[i] = thirdRailBottom[i - 14];
			}

			if (TrailLeft[i] == 0 && TrailTop[i] == 0 && TrailRight[i] == 0 && TrailBottom[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posY = 160;
		break;
	default:
		return 2;
		break;
	}

	lineSelect = type - 1;

	if (0 == lineSelect)
	{
		for (int i = 0; i <= count; i++)
		{
			tmpTrailLeft[i] = TrailLeft[i];
			tmpTrailTop[i] = (TrailTop[i] + 30);
			tmpTrailRight[i] = TrailRight[i];
			tmpTrailBottom[i] = (TrailBottom[i] + 30);
		}
	}
	else {
		for (int i = 0; i <= count; i++)
		{
			tmpTrailLeft[i] = TrailLeft[i];
			tmpTrailTop[i] = TrailTop[i];
			tmpTrailRight[i] = TrailRight[i];
			tmpTrailBottom[i] = TrailBottom[i];
		}
	}

	dc.Attach(hdc);

	while (TRUE) {
		Sleep(10);	//이동 딜레이

		if (cirCount == cirCountText)	// 순환 종료 및 반환
		{
			InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			break;
		}

		if (isReverse && changeDirection[2])	// 방향 전환 : 역방향
		{
			for (int i = 0; i <= count; i++)
			{
				TrailLeft[i] = tmpTrailLeft[count - i];
				TrailTop[i] = tmpTrailTop[count - i];
				TrailRight[i] = tmpTrailRight[count - i];
				TrailBottom[i] = tmpTrailBottom[count - i];
			}
			changeDirection[2] = FALSE;
		}
		else if (!isReverse && changeDirection[2])	// 방향 전환 : 정방향
		{
			if (0 == lineSelect)
			{
				for (int i = 0; i <= count; i++)
				{
					TrailLeft[i] = tmpTrailLeft[i];
					TrailTop[i] = tmpTrailTop[i] - 30;
					TrailRight[i] = tmpTrailRight[i];
					TrailBottom[i] = tmpTrailBottom[i] - 30;
				}
			}
			else
			{
				for (int i = 0; i <= count; i++)
				{
					TrailLeft[i] = tmpTrailLeft[i];
					TrailTop[i] = tmpTrailTop[i];
					TrailRight[i] = tmpTrailRight[i];
					TrailBottom[i] = tmpTrailBottom[i];
				}
			}
			changeDirection[2] = FALSE;
		}

		switch (flag)	//열차 이동
		{
		case 1:	//오른
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, train, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 2:	//아래
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			InvalidateRect(pArg->hwnd, train, TRUE);
			trainY = trainSpeed + trainHeight;
			break;
		case 3:	//왼
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, train, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 4:	//위
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			InvalidateRect(pArg->hwnd, train, TRUE);
			trainY = trainSpeed + trainHeight;
			break;
		default:
			OutputDebugStringW(_T("\r\nCTrain >> DrawObject >> Out of Flag Range\r\n"));
			dc.Detach();
			::ReleaseDC(pArg->hwnd, hdc);
			return 2;
			break;
		}

		stationRect = CRect(TrailLeft[stationCount], TrailTop[stationCount], TrailRight[stationCount], TrailBottom[stationCount]);
		subStationRect = CRect(TrailLeft[subStationCount], TrailTop[subStationCount], TrailRight[subStationCount], TrailBottom[subStationCount]);

		IntersectRect(tmpRect, train, stationRect) && stationCount >= 0 ? insCheck[lineSelect][stationCount] = TRUE : NULL;

		dc.Rectangle(train);	//열차 그리기
		testStr.Format(_T("%d %d || %d %d || %d %d\n"), train.left, train.top, stationRect.left, stationRect.top, subStationRect.right, subStationRect.bottom);
		OutputDebugStringW(testStr);

		if (IntersectRect(tmpRect, train, stationRect) && stationCount >= 0)
		{
			DrawFillRect(param, stationRect, 0, 255, 0);
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if (!insCheck[lineSelect][subStationCount] && stationCount >= 1 && enableInvalid) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				enableInvalid = FALSE;
			}
		}
		else if (IntersectRect(tmpRect, train, subStationRect) && subStationCount >= 0)
		{
			DrawFillRect(param, subStationRect, 255, 0, 0);
			insCheck[lineSelect][subStationCount] = FALSE;
			enableInvalid = TRUE;
		}

		if (stationCount == RAIL_NUM)	// 이동 종료
		{
			if (cycleEnable)
			{
				isReverse = isReverse ? FALSE : TRUE;
				stationCount = 1;
				subStationCount = 0;
				trainSpeed = trainX - trainWidth;
				changeDirection[2] = TRUE;
				cirCount++;
				if (0 == lineSelect)
				{
					posY = isReverse ? 30 : 0;
					InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				}
				
			}
			else
			{
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				break;
			}
			
		}
		else if (stationCount == 0 && !isReverse)	// 생성 후 이동
		{
			moveEnable = insCheck[lineSelect][stationCount + 1] || startInsCheck[lineSelect] ? FALSE : TRUE;
			if ((1 == flag || 3 == flag) && (trainWidth + trainSpeed + posX) == (stationRect.right - 20) && moveEnable)	// 좌, 우
			{
				subStationCount = stationCount;
				startInsCheck[lineSelect] = TRUE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				Sleep(1000);
				stationCount++;
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed + posY) == (stationRect.bottom - 10) && moveEnable)	// 상, 하
			{
				subStationCount = stationCount;
				startInsCheck[lineSelect] = TRUE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				Sleep(1000);
				stationCount++;
			}

		}
		else if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)	// 상, 하 이동
		{
			changeDirection[1] = TRUE;

			if (changeDirection[0])	// 좌, 우 이동에서 넘어오면 값 조정
			{
				trainSpeed = trainY - trainHeight;
				changeDirection[0] = FALSE;
			}

			flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T(2) : 하, F(4) : 상
			moveEnable = insCheck[lineSelect][stationCount + 1] ? FALSE : TRUE;

			if ((trainHeight + trainSpeed + posY) == (stationRect.bottom - 10))	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				Sleep(1000);
				stationCount++;
			}
		}
		else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)	// 좌, 우 이동
		{
			changeDirection[0] = TRUE;

			if (changeDirection[1])	// 상, 하 이동에서 넘어오면 값 조정
			{
				trainSpeed = trainX - trainWidth;
				changeDirection[1] = FALSE;
			}

			flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T(3) : 좌, F(1) : 우
			moveEnable = insCheck[lineSelect][stationCount + 1] ? FALSE : TRUE;

			if ((trainWidth + trainSpeed + posX + 20) == stationRect.right)	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				Sleep(1000);
				OutputDebugStringW(_T("===================================================\n"));
				stationCount++;
			}
		}
		else // 사용하지 않는 개수 증가
		{
			stationCount++;
		}

		stationCount == 2 ? startInsCheck[lineSelect] = FALSE : NULL;	// 초기 시작 충돌 방지
	}
	
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
	return id;
}

UINT TestDrawObject(LPVOID param, int type, UINT cycleCount, BOOL checkCycleEnable, UINT id)	//그리기 및 이동 테스트용
{
	CTrain* pMain = (CTrain*)param;
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;				//열차
	int trainSpeed = 0;		//열차 속도
	int trainX = 50;		//x기억
	int trainY = 30;		//y기억
	int trainWidth = 50;	//열차 가로 길이
	int trainHeight = 15;	//열차 세로 길이
	int flag = 1;			//열차 이동 방향		1 : Right / 2 : Down / 3 : Left / 4 : Up
	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재 도착역
	int subStationCount = 0;//정차이후 출발역
	int lineSelect = 1;		//선로 번호저장

	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역

	CRect tmpRect;			//곂치는 영역 저장
	BOOL moveEnable = TRUE;	//이동 가능 여부
	BOOL changeDirection[3] = { TRUE, TRUE, FALSE };	//1,2 : 방향 전환 여부(좌,우 <-> 상,하), 3 : 정방향, 역방향 여부(좌표 변경용)
	BOOL isReverse = FALSE;	//정방향, 역방향 여부(방향 전환용)

	// 선로 좌표 저장
	int TrailLeft[RAIL_NUM] = { 0 };
	int TrailTop[RAIL_NUM] = { 0 };
	int TrailRight[RAIL_NUM] = { 0 };
	int TrailBottom[RAIL_NUM] = { 0 };

	// 임시 선로 좌표 저장
	int tmpTrailLeft[RAIL_NUM] = { 0 };
	int tmpTrailTop[RAIL_NUM] = { 0 };
	int tmpTrailRight[RAIL_NUM] = { 0 };
	int tmpTrailBottom[RAIL_NUM] = { 0 };

	CString testStr;		///Test String
	int cirCount = 0;		//순환 횟수(증가용)
	int cirCountText = 1;	//순환 횟수(저장용)
	BOOL cirEnable = FALSE;	//순환 가능 여부

	cirCountText = (cycleCount * 2);
	cirEnable = checkCycleEnable;
	
	int arraySize = (sizeof(firstRailLeft) / sizeof(*firstRailLeft)) / 2;
	for (int i = 0; i < arraySize; i++)
	{
		TrailLeft[i] = firstRailLeft[i];
		TrailTop[i] = 10;
		TrailRight[i] = firstRailLeft[i + 1];
		TrailBottom[i] = 30;
	}

	for (int i = 0; i < arraySize; i++)
	{
		tmpTrailLeft[i] = firstRailLeft[i];
		tmpTrailTop[i] = 40;
		tmpTrailRight[i] = firstRailLeft[i + 1];
		tmpTrailBottom[i] = 60;
	}
	arraySize--;

	rect = CRect(trainSpeed + posX, trainY - trainHeight, trainSpeed + trainWidth, trainY);	//열차 초기 위치 설정
	dc.Attach(hdc);

	while (TRUE) {
		Sleep(10);	//이동 딜레이

		if (cirCount == cirCountText)	// 순환 종료 및 반환
		{
			testStr.Format(_T("%d %d  ||  %d %d\n"), stationRect.left, stationRect.right, subStationRect.left, subStationRect.right);
			OutputDebugStringW(testStr);
			InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			break;
		}

		if (isReverse && changeDirection[2])	// 방향 전환 : 역방향
		{
			for (int i = 0; i <= arraySize; i++)
			{
				TrailLeft[i] = tmpTrailLeft[arraySize - i];
				TrailTop[i] = tmpTrailTop[arraySize - i];
				TrailRight[i] = tmpTrailRight[arraySize - i];
				TrailBottom[i] = tmpTrailBottom[arraySize - i];
			}
			moveEnable = FALSE;
			changeDirection[2] = FALSE;
		}
		else if (!isReverse && changeDirection[2])	// 방향 전환 : 정방향
		{
			for (int i = 0; i <= arraySize; i++)
			{
				TrailLeft[i] = tmpTrailLeft[i];
				TrailTop[i] = tmpTrailTop[i];
				TrailRight[i] = tmpTrailRight[i];
				TrailBottom[i] = tmpTrailBottom[i];
			}
			cirCount++;
			changeDirection[2] = FALSE;
		}

		switch (flag)	//열차 이동
		{
		case 1:	//오른
			moveEnable ? trainSpeed += 5 : Sleep(10);
			rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 2:	//아래
			moveEnable ? trainSpeed += 5 : Sleep(10);
			rect = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainY = trainSpeed + trainHeight;
			break;
		case 3:	//왼
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 4:	//위
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			rect = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainY = trainSpeed + trainHeight;
			break;
		default:
			OutputDebugStringW(_T("\r\nCTrain >> DrawObject >> Out of Flag Range\r\n"));
			return 2;
			break;
		}

		stationRect = CRect(TrailLeft[stationCount], TrailTop[stationCount], TrailRight[stationCount], TrailBottom[stationCount]);					//현재역 영역
		subStationRect = CRect(TrailLeft[subStationCount], TrailTop[subStationCount], TrailRight[subStationCount], TrailBottom[subStationCount]);	//이전역 영역

		IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0 ? insCheck[lineSelect][stationCount] = TRUE : NULL;

		CBrush brush;
		CBrush* oldBrush = dc.SelectObject(&brush);

		dc.SelectObject(oldBrush);
		dc.Rectangle(rect);	//열차 그리기
		dc.DrawText(_T("1002"), rect, DT_NOCLIP | DT_CENTER);

		if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0)
		{
			//색칠 + 무효화
			brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
			oldBrush = dc.SelectObject(&brush);
			dc.Rectangle(stationRect);
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if (!insCheck[lineSelect][subStationCount] && stationCount >= 1) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			}

		}
		else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
		{
			//색만 칠하기
			brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
			oldBrush = dc.SelectObject(&brush);
			dc.Rectangle(subStationRect);
			insCheck[lineSelect][subStationCount] = FALSE;

		}

		if (stationCount == RAIL_NUM)	// 이동 종료
		{
			if (cirEnable)
			{
				isReverse = isReverse ? FALSE : TRUE;
				stationCount = 1;
				subStationCount = 0;
				trainSpeed = trainX - trainWidth;
				changeDirection[2] = TRUE;
				posY = 30;
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				cirCount++;
			}
			else
			{
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				break;
			}

		}
		else if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)	// 상, 하 이동
		{
			changeDirection[1] = TRUE;

			if (changeDirection[0])
			{
				trainSpeed = trainY - trainHeight;
				changeDirection[0] = FALSE;
			}

			flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T(2) : 하, F(4) : 상
			moveEnable = insCheck[lineSelect][stationCount + 1] ? FALSE : TRUE;

			if ((trainHeight + trainSpeed + posY) == (stationRect.bottom - 10))	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				Sleep(1000);
				stationCount++;
			}
		}
		else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)	// 좌, 우 이동
		{
			changeDirection[0] = TRUE;

			if (changeDirection[1])
			{
				trainSpeed = trainX - trainWidth;
				changeDirection[1] = FALSE;
			}

			flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T(3) : 좌, F(1) : 우
			moveEnable = insCheck[lineSelect][stationCount + 1] ? FALSE : TRUE;

			if ((trainWidth + trainSpeed + posX + 20) == stationRect.right)	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				Sleep(1000);
				stationCount++;
			}
		}
		else if (stationCount == 0 && !isReverse)	// 생성 후 이동
		{
			moveEnable = insCheck[lineSelect][stationCount + 1] || startInsCheck[lineSelect] ? FALSE : TRUE;
			if ((1 == flag || 3 == flag) && (trainWidth + trainSpeed + posX + 20) == stationRect.right && moveEnable)	// 좌, 우
			{
				subStationCount = stationCount;
				startInsCheck[lineSelect] = TRUE;
				Sleep(1000);
				stationCount++;
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed + posY) == (stationRect.bottom - 10) && moveEnable)	// 상, 하
			{
				subStationCount = stationCount;
				startInsCheck[lineSelect] = TRUE;
				Sleep(1000);
				stationCount++;
			}

		}
		else // 사용하지 않는 개수 증가
		{
			stationCount++;
		}

		stationCount == 2 ? startInsCheck[lineSelect] = FALSE : NULL;	// 초기 시작 충돌 방지
	}

	return id;
}

UINT ThreadMoveTrain(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	ThreadArg* pArg = (ThreadArg*)param;
	CBrush brush = RGB(255, 255, 255);
	int errorCode = 10000;
	DWORD dwResult;

	if (pArg->type > 0 && pArg->type < 5)
	{
		errorCode = DrawObject(pMain, pArg->type, pArg->cycleCount, pArg->checkCycleEnable, pArg->id);
	}
	else if (100 == pArg->type)
	{
		errorCode = 100;
		TestDrawObject(pMain, pArg->type, pArg->cycleCount, pArg->checkCycleEnable, pArg->id);
	}
	else
	{
		errorCode = 2;
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
	}

	if (errorCode >= 1000 && errorCode < 10000)
	{
		::GetExitCodeThread(pMain->m_thread_move[(errorCode - 1000)], &dwResult);
		trainCount--;
	}

	return 0;
}

/*
HBRUSH CTrain::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	HBRUSH hbr;


	if (nCtlColor == CTLCOLOR_STATIC)	//STATIC 컨트롤만 배경색 변경
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 변경

	hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;

}
*/
