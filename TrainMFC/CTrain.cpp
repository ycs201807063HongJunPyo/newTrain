// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

#define TRAIN_TIMER 10
#define WAIT_TIMER 11
// CTrain 대화 상자

IMPLEMENT_DYNAMIC(CTrain, CDialog)

//열차 구역
int railTopLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };		// 좌
int railTopRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };	// 우
int railBottomLeft[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };			// 상
int railBottomRight[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };		// 하

int testRailTopLeft[RAIL_NUM] = { 520, 620, 720, 820, 820, 920, 920, 920, 820, 720, 720, 620, 520 };
int testRailTopRight[RAIL_NUM] = { 620, 720, 820, 920, 920, 1020, 1020, 1020, 920, 820, 820, 720, 620 };
int testRailBottomLeft[RAIL_NUM] = { 10,10,10,10, 60, 60, 110, 160, 160, 160, 110, 110, 110 };
int testRailBottomRight[RAIL_NUM] = { 60,60,60,60, 110, 110, 160, 210, 210, 210, 160, 160, 160 };

//BOOL insCheck[RAIL_NUM] = { FALSE, };
BOOL testInsCheck[RAIL_NUM] = { FALSE, };
//겹치는 구역
BOOL rectResult;
CRect testRect;
//열차 출발/정지
BOOL trainFlag;
UINT numLine;


CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	startFlag = TRUE;

	trainCount = 0;
}

CTrain::~CTrain()
{
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, testText1);
	DDX_Control(pDX, IDC_STATIC2, testText2);
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
	CString strLine = _T("");

	trainFlag = TRUE;

	GetDlgItemText(IDC_EDIT_LINE, strLine);
	numLine = _wtoi(strLine);
	
	arg1.hwnd = this->m_hWnd;
	arg1.type = numLine;
	if (trainCount <= 2) {
		m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &arg1, THREAD_PRIORITY_NORMAL, 0, 0);
		trainCount++;
	}
}

void CTrain::OnBnClickedStart()
{
	trainFlag = TRUE;

	for (int i = 0; i < trainCount; i++)
	{
		m_thread_move[i]->ResumeThread();
	}

	//SetTimer(TRAIN_TIMER, 50, NULL);
}


void CTrain::OnBnClickedStop()
{
	trainFlag = FALSE;

	for (int i = 0; i < trainCount; i++)
	{
		m_thread_move[i]->SuspendThread();
	}
}

void CTrain::TimerFunction(int flag) {
	//열차 대기
	if (flag == 0) {
		KillTimer(TRAIN_TIMER);
		trainFlag = FALSE;
		SetTimer(WAIT_TIMER, 500, NULL);
	}
	//열차 출발
	else if (flag == 1) {
		KillTimer(WAIT_TIMER);
		trainFlag = TRUE;
		SetTimer(TRAIN_TIMER, 50, NULL);
	}

}

void CTrain::TrainMove(int left, int top, int right, int bottom) {
	//가상열차 위치 지정
	trainSize.left = left;
	trainSize.right = right;
	trainSize.top = top;
	trainSize.bottom = bottom;
}


void CTrain::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CDC MemDC;

	// 화면 DC와 호환되는 메모리 DC를 생성
	MemDC.CreateCompatibleDC(&dc);
	// 비트맵 리소스 로딩
	CBitmap bmp;
	CBitmap* pOldBmp = NULL;

	//맵 만들기
	MemDC.SelectObject(pOldBmp);
	//역 테두리 설정
	CPen myPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = dc.SelectObject(&myPen);
	int arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(railTopLeft[i], railBottomLeft[i], railTopRight[i], railBottomRight[i]);
	}
	for (int i = 0; i < arraySize; i++)
	{
		dc.Rectangle(testRailTopLeft[i], testRailBottomLeft[i], testRailTopRight[i], testRailBottomRight[i]);
	}
	dc.SelectObject(oldPen);


	//출발 초록색 표시
	CBrush brush;
	CBrush* oldBrush = dc.SelectObject(&brush);
	brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
	oldBrush = dc.SelectObject(&brush);
	//	dc.Rectangle(railTopLeft[subStationCount], railBottomLeft[subStationCount], railTopRight[subStationCount], railBottomRight[subStationCount]);

		//dc.Rectangle(testRect.left, testRect.top, testRect.right, testRect.bottom);
	dc.SelectObject(oldBrush);    // 시스템 브러시 객체를 돌려줌

	MemDC.SelectObject(pOldBmp);


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
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;				//열차
	int trainSpeed = 0;		//열차 속도
	int trainX = 0;			//x기억
	int trainY = 50;		//y기억
	int trainWidth = 30;	//열차 가로 길이
	int trainHeight = 30;	//열차 세로 길이
	int flag = 1;			//열차 방향
	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재 도착역
	int subStationCount = 0; //정차이후 출발역
	int arraySize = 0;

	CRect tmpRect;			//곂치는 영역 저장용
	BOOL moveEnable = TRUE;
	BOOL changeDirection[2] = { FALSE, FALSE };
	
	int TrailTopLeft[RAIL_NUM] = { 0 };
	int TrailTopRight[RAIL_NUM] = { 0 };
	int TrailBottomLeft[RAIL_NUM] = { 0 };
	int TrailBottomRight[RAIL_NUM] = { 0 };

	switch (type)
	{
	case 2:
		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
		for (int i = 0; i < arraySize; i++)
		{
			TrailTopLeft[i] = railTopLeft[i];
			TrailTopRight[i] = railTopRight[i];
			TrailBottomLeft[i] = railBottomLeft[i];
			TrailBottomRight[i] = railBottomRight[i];
		}
		break;
	case 3:
		arraySize = (sizeof(testRailBottomLeft) / sizeof(*testRailBottomLeft));
		for (int i = 0; i < arraySize; i++)
		{
			TrailTopLeft[i] = testRailTopLeft[i];
			TrailTopRight[i] = testRailTopRight[i];
			TrailBottomLeft[i] = testRailBottomLeft[i];
			TrailBottomRight[i] = testRailBottomRight[i];
		}
		posX = 500;
		break;
	default:
		return 2;
		break;
	}

	rect = CRect(trainSpeed + posX, trainY - trainHeight, trainSpeed + trainWidth, trainY);	//열차 초기 위치 설정
	dc.Attach(hdc);

	while (TRUE) {
		if (trainFlag == TRUE) {
			Sleep(10);

			switch (flag)	//열차 이동
			{
			case 1:	//오른
				moveEnable ? trainSpeed += 10 : NULL;
				rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainX = trainSpeed + trainWidth;
				break;
			case 2:	//아래
				moveEnable ? trainSpeed += 10 : NULL;
				rect = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainY = trainSpeed + trainHeight;
				break;
			case 3:	//왼
				moveEnable ? trainSpeed -= 10 : NULL;
				rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainX = trainSpeed + trainWidth;
				break;
			case 4:	//위
				moveEnable ? trainSpeed -= 10 : NULL;
				rect = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + 30 + posY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainY = trainSpeed + trainHeight;
				break;
			default:
				OutputDebugStringW(_T("\r\nCTrain >> DrawObject >> Out of Flag Range\r\n"));
				return 2;
				break;
			}
/*
			if (IntersectRect(tmpRect, rect, CRect(railTopLeft[stationCount], railBottomLeft[stationCount], railTopRight[stationCount], railBottomRight[stationCount])) && stationCount >= 0)
			{
				insCheck[stationCount] = TRUE;
			}
*/
			CBrush brush;
			CBrush* oldBrush = dc.SelectObject(&brush);
			CRect stationRect;		//현재역 영역
			CRect subStationRect;	//이전역 영역

			dc.SelectObject(oldBrush);
			dc.Rectangle(rect);	//열차 그리기

			stationRect = CRect(TrailTopLeft[stationCount], TrailBottomLeft[stationCount], TrailTopRight[stationCount], TrailBottomRight[stationCount]);
			subStationRect = CRect(TrailTopLeft[subStationCount], TrailBottomLeft[subStationCount], TrailTopRight[subStationCount], TrailBottomRight[subStationCount]);

			if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0)
			{
				//색칠 + 무효화
				brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
				oldBrush = dc.SelectObject(&brush);
				dc.Rectangle(stationRect);
				//이전 부분과 충돌이 있을경우에만 무효화 해주기
//				if (IntersectRect(tmpRect, rect, CRect(railTopLeft[subStationCount], railBottomLeft[subStationCount], railTopRight[subStationCount], railBottomRight[subStationCount])) && !insCheck[subStationCount]) {
				if (IntersectRect(tmpRect, rect, subStationRect) && stationCount >= 1) {
					InvalidateRect(pArg->hwnd, CRect(TrailTopLeft[stationCount - 1], TrailBottomLeft[stationCount - 1], TrailTopRight[stationCount - 1], TrailBottomRight[stationCount - 1]), TRUE);
				}

			}
			else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
			{
				//색만 칠하기
				brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
				oldBrush = dc.SelectObject(&brush);
				dc.Rectangle(subStationRect);
//				insCheck[subStationCount] = FALSE;

			}

			// 위, 아래 이동
			if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)
			{
				changeDirection[1] = TRUE;

				if (changeDirection[0])
				{
					trainSpeed = trainY - 30;
					changeDirection[0] = FALSE;
				}

				flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T : 하, F : 상
				moveEnable = TRUE;

				if ((30 + trainSpeed) == (stationRect.bottom - 10))
				{
					subStationCount = stationCount;
					moveEnable = FALSE;
					Sleep(1000);
					stationCount++;
				}
			}

			// 좌, 우 이동
			else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)
			{
				changeDirection[0] = TRUE;

				if (changeDirection[1])
				{
					trainSpeed = trainX - 30;
					changeDirection[1] = FALSE;
				}

				flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T : 좌, F : 우
				moveEnable = TRUE;

				if ((50 + trainSpeed + posX) == stationRect.right)
				{
					subStationCount = stationCount;
					moveEnable = FALSE;
					Sleep(1000);
					stationCount++;
				}
			}

			// 생성 후 이동
			else if (stationCount == 0)
			{
				if ((1 == flag || 3 == flag) && (50 + trainSpeed + posX) == stationRect.right)
				{
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
				}
				else if ((2 == flag || 4 == flag) && (30 + trainSpeed) == (stationRect.bottom - 10))
				{
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
				}

			}
			
			/*
			switch (type)
			{
			case 2:
				//정차, 방향 조정
				if (stationCount <= 4 && (50 + trainSpeed + posX) == stationRect.right) {
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
					if (stationCount == 5) {
						trainSpeed = 20;
						flag = 2;
					}
				}
				else if ((stationCount >= 5 && stationCount <= 6 && (30 + trainSpeed + posY) == stationRect.bottom - 10)) {
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
					if (stationCount == 7) {
						trainSpeed = trainX - 30;
						flag = 3;
					}
				}
				else if (stationCount >= 5 && stationCount <= 10 && (70 + trainSpeed + posX) == stationRect.right) {
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
					if (stationCount == 11) {
						trainSpeed = trainY - 30;
						flag = 4;
					}
				}
				else if ((stationCount >= 11 && stationCount <= 12 && (30 + trainSpeed + posY) == stationRect.bottom - 10)) {
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
					if (stationCount == 13) {
						stationCount = 1;
						trainSpeed = 50;
						trainX = 0;
						trainY = 50;
						flag = 1;
					}
				}
				break;
			case 3:
				//정차, 방향 조정
				if (stationCount <= 3 && (50 + trainSpeed + posX) == testRailTopRight[stationCount]) {
					subStationCount = stationCount;
					flag = 1;
					Sleep(1000);
					stationCount++;
					if (stationCount == 4) {
						trainSpeed = 20;
						flag = 2;
					}
				}
				else if ((stationCount == 4 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10)) {
					subStationCount = stationCount;
					flag = 2;
					Sleep(1000);
					stationCount++;
					if (stationCount == 5) {
						trainSpeed = trainX - 30;
						flag = 1;
					}
				}
				else if (stationCount == 5 && (50 + trainSpeed + posX) == testRailTopRight[stationCount]) {
					subStationCount = stationCount;
					flag = 1;
					Sleep(1000);
					stationCount++;
					if (stationCount == 6) {
						trainSpeed = trainY - 30;
						flag = 2;
					}
				}
				else if ((stationCount >= 6 && stationCount <= 7 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10)) {
					subStationCount = stationCount;
					flag = 2;
					Sleep(1000);
					stationCount++;
					if (stationCount == 8) {
						trainSpeed = trainX - 30;
						flag = 3;
					}
				}
				else if ((stationCount >= 8 && stationCount <= 9 && (50 + trainSpeed + posX) == testRailTopRight[stationCount])) {
					subStationCount = stationCount;
					flag = 3;
					Sleep(1000);
					stationCount++;
					if (stationCount == 10) {
						trainSpeed = trainY - 30;
						flag = 4;
					}
				}
				else if (stationCount == 10 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10) {
					subStationCount = stationCount;
					flag = 4;
					Sleep(1000);
					stationCount++;
					if (stationCount == 11) {
						trainSpeed = trainX - 30;
						flag = 3;
					}
				}
				else if (stationCount >= 11 && stationCount <= 13 && (50 + trainSpeed + posX) == testRailTopRight[stationCount]) {
					subStationCount = stationCount;
					flag = 3;
					Sleep(1000);
					stationCount++;
					if (stationCount == 13) {
						InvalidateRect(pArg->hwnd, CRect(TrailTopLeft[stationCount - 1], TrailBottomLeft[stationCount - 1], TrailTopRight[stationCount - 1], TrailBottomRight[stationCount - 1]), TRUE);
						stationCount = 0;
						trainSpeed = 50;
						trainX = 0;
						trainY = 50;
						flag = 1;
					}
				}
				break;
			default:
				OutputDebugStringW(_T("\r\nCTrain >> DrawObject >> Out of Flag Range\r\n"));
				return 2;
				break;
			}
			*/
		}
	}

	return 1;
}

UINT TestDrawObject(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;
	int trainSpeed = 0;  //열차 속도
	int trainX;  //x기억
	int trainY = 50;  //y기억
	int flag = 1;

	CRect MainRectTest;
	CRect tmpRect;
	CRect testtmpRect;
	BOOL testRectResult;
	BOOL TestMove = TRUE;

	BOOL changeDirection[2] = { FALSE, FALSE };

	rect = CRect(trainSpeed + 500, 20, 30 + trainSpeed + 500, 50);
	dc.Attach(hdc);

	int stationCount = 0; //현재 도착역
	int subStationCount = 0; //정차이후 출발역

	while (1) {
		if (trainFlag == TRUE) {
			Sleep(10);
			if (stationCount <= 7 && TestMove) {
				trainSpeed += 10;
			}
			else if(TestMove) {
				trainSpeed -= 10;
			}
			testRectResult = testtmpRect.IntersectRect(&rect, &CRect(testRailTopLeft[stationCount], testRailBottomLeft[stationCount], testRailTopRight[stationCount], testRailBottomRight[stationCount]));

			switch (flag)
			{
			case 1:
				//오른
				rect = CRect(trainSpeed + 500, trainY - 30, 30 + trainSpeed + 500, trainY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainX = trainSpeed + 30;
				break;
			case 2:
				//아래
				rect = CRect(trainX - 30 + 500, trainSpeed, trainX + 500, 30 + trainSpeed);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainY = trainSpeed + 30;
				break;
			case 3:
				//왼
				rect = CRect(trainSpeed + 500, trainY - 30, trainSpeed + 30 + 500, trainY);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainX = trainSpeed + 30;
				break;
			case 4:
				//위
				rect = CRect(trainX - 30 + 500, trainSpeed, trainX + 500, 30 + trainSpeed);
				InvalidateRect(pArg->hwnd, rect, TRUE);
				trainY = trainSpeed + 30;
				break;
			default:
				break;
			}

			CBrush brush;
			CBrush* oldBrush = dc.SelectObject(&brush);

			if (IntersectRect(tmpRect, rect, CRect(testRailTopLeft[stationCount], testRailBottomLeft[stationCount], testRailTopRight[stationCount], testRailBottomRight[stationCount])) && stationCount >= 0)
			{
				testInsCheck[stationCount] = TRUE;
			}
			dc.SelectObject(oldBrush);

			dc.Rectangle(rect);

			MainRectTest = rect;

			CString tmp;
			tmp.Format(_T("%d %d %d %d %d\n"), rect.left, rect.top, subStationCount, stationCount, flag);
			OutputDebugStringW(tmp);

			if (IntersectRect(tmpRect, MainRectTest, CRect(testRailTopLeft[stationCount], testRailBottomLeft[stationCount], testRailTopRight[stationCount], testRailBottomRight[stationCount])) && stationCount >= 0)
			{
				//색칠 + 무효화
				brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
				oldBrush = dc.SelectObject(&brush);
				dc.Rectangle(testRailTopLeft[stationCount], testRailBottomLeft[stationCount], testRailTopRight[stationCount], testRailBottomRight[stationCount]);
				//이전 부분과 충돌이 있을경우에만 무효화 해주기
				if (IntersectRect(tmpRect, MainRectTest, CRect(testRailTopLeft[subStationCount], testRailBottomLeft[subStationCount], testRailTopRight[subStationCount], testRailBottomRight[subStationCount])) && !testInsCheck[subStationCount] && stationCount >= 1) {
					InvalidateRect(pArg->hwnd, CRect(testRailTopLeft[stationCount - 1], testRailBottomLeft[stationCount - 1], testRailTopRight[stationCount - 1], testRailBottomRight[stationCount - 1]), TRUE);
				}

			}
			else if (IntersectRect(tmpRect, MainRectTest, CRect(testRailTopLeft[subStationCount], testRailBottomLeft[subStationCount], testRailTopRight[subStationCount], testRailBottomRight[subStationCount])) && subStationCount >= 0)
			{
				//색만 칠하기
				brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
				oldBrush = dc.SelectObject(&brush);
				dc.Rectangle(testRailTopLeft[subStationCount], testRailBottomLeft[subStationCount], testRailTopRight[subStationCount], testRailBottomRight[subStationCount]);
				testInsCheck[subStationCount] = FALSE;

			}

			// 위, 아래 이동
			if (testRailTopLeft[stationCount] == testRailTopLeft[subStationCount] && testRailBottomLeft[stationCount] != testRailBottomLeft[subStationCount] && stationCount >= 1)
			{
				changeDirection[1] = TRUE;

				if (changeDirection[0])
				{
					trainSpeed = trainY - 30;
					changeDirection[0] = FALSE;
				}

				flag = testRailBottomLeft[stationCount] != testRailBottomLeft[subStationCount] && testRailBottomLeft[stationCount] > testRailBottomLeft[subStationCount] ? 2 : 4;	// T : 하, F : 상
				TestMove = TRUE;
				
				if ((30 + trainSpeed) == testRailBottomRight[stationCount] - 10)
				{
					subStationCount = stationCount;
					TestMove = FALSE;
					Sleep(1000);
					stationCount++;
				}
			}

			// 좌, 우 이동
			else if (testRailTopLeft[stationCount] != testRailTopLeft[subStationCount] && testRailBottomLeft[stationCount] == testRailBottomLeft[subStationCount] && stationCount >= 1)
			{
				changeDirection[0] = TRUE;

				if (changeDirection[1])
				{
					trainSpeed = trainX - 30;
					changeDirection[1] = FALSE;
				}

				flag = testRailTopLeft[stationCount] != testRailTopLeft[subStationCount] && testRailTopLeft[stationCount] < testRailTopLeft[subStationCount] ? 3 : 1;	// T : 좌, F : 우
				TestMove = TRUE;

				if ((50 + trainSpeed + 500) == testRailTopRight[stationCount])
				{
					subStationCount = stationCount;
					TestMove = FALSE;
					Sleep(1000);
					stationCount++;
				}
			}

			// 생성 후 이동
			else if (stationCount == 0)
			{
				if ((1 == flag || 3 == flag) && (50 + trainSpeed + 500) == testRailTopRight[stationCount])
				{
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
				}
				else if ((2 == flag || 4 == flag) && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10)
				{
					subStationCount = stationCount;
					Sleep(1000);
					stationCount++;
				}
				
			}

			/*
			//정차, 방향 조정
			if (stationCount <= 3 && (50 + trainSpeed + 500) == testRailTopRight[stationCount]) {
				subStationCount = stationCount;
				flag = 1;
				Sleep(1000);
				stationCount++;
				if (stationCount == 4) {
					trainSpeed = 20;
					flag = 2;
				}
			}
			else if ((stationCount == 4 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10)) {
				subStationCount = stationCount;
				flag = 2;
				Sleep(1000);
				stationCount++;
				if (stationCount == 5) {
					trainSpeed = trainX - 30;
					flag = 1;
				}
			}
			else if (stationCount == 5 && (50 + trainSpeed + 500) == testRailTopRight[stationCount]) {
				subStationCount = stationCount;
				flag = 1;
				Sleep(1000);
				stationCount++;
				if (stationCount == 6) {
					trainSpeed = trainY - 30;
					flag = 2;
				}
			}
			else if ((stationCount >= 6 && stationCount <= 7 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10)) {
				subStationCount = stationCount;
				flag = 2;
				Sleep(1000);
				stationCount++;
				if (stationCount == 8) {
					trainSpeed = trainX - 30;
					flag = 3;
				}
			}
			else if ((stationCount >= 8 && stationCount <= 9 && (50 + trainSpeed + 500) == testRailTopRight[stationCount])) {
				subStationCount = stationCount;
				flag = 3;
				Sleep(1000);
				stationCount++;
				if (stationCount == 10) {
					trainSpeed = trainY - 30;
					flag = 4;
				}
			}
			else if (stationCount == 10 && (30 + trainSpeed) == testRailBottomRight[stationCount] - 10) {
				subStationCount = stationCount;
				flag = 4;
				Sleep(1000);
				stationCount++;
				if (stationCount == 11) {
					trainSpeed = trainX - 30;
					flag = 3;
				}
			}
			else if (stationCount >= 11 && stationCount <= 13 && (50 + trainSpeed + 500) == testRailTopRight[stationCount]) {
				subStationCount = stationCount;
				flag = 3;
				Sleep(1000);
				stationCount++;
				if (stationCount == 13) {
					stationCount = 1;
					trainSpeed = 50;
					trainX = 0;
					trainY = 50;
				}
			}
			*/
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
	int errorCode = 0;

	dc.Attach(hdc);
	dc.SelectObject(&brush);

	switch (pArg->type)
	{
	case 2:
		dc.SelectObject(&brush);
		while (0 != DrawObject(pMain, pArg->type))
		{
			DrawObject(pMain, pArg->type);
		}
		break;
	case 3:
		dc.SelectObject(&brush);
		while (0 != DrawObject(pMain, pArg->type))
		{
			DrawObject(pMain, pArg->type);
		}
		break;
	case 4:
		dc.SelectObject(&brush);
		while (0 != TestDrawObject(pMain))
		{
			TestDrawObject(pMain);
		}
		break;
	default:
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
		break;
	}

	switch (errorCode)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
		break;
	default:
		break;
	}

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);

	return 0;
}


/*
CString test;
			test.Format(L"%d", bmpInfo.bmHeight + trainSpeed);
			subStationCountext1.SetWindowTextW(test);
			test.Format(L"%d", trainCount);
			subStationCountext2.SetWindowTextW(test);
*/

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
