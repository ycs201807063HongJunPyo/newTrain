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

//열차 구역 내선
int railTopLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };
int railTopRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };
int railBottomLeft[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };
int railBottomRight[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };
//열차 구역 외선
int smallRailTopLeft[3] = { 700, 510, 410 };
int smallRailTopRight[3] = { 900, 700, 510 };
int smallRailBottomLeft[3] = { 60, 60, 60 };
int smallRailBottomRight[3] = { 110, 110, 110 };
//열차 구역 반복
int subRailTopLeft[3] = { 850, 850, 850 };
int subRailTopRight[3] = { 900, 900, 900 };
int subRailBottomLeft[3] = { 310, 210, 110 };
int subRailBottomRight[3] = { 410, 310, 210 };
BOOL insCheck[RAIL_NUM] = { FALSE, };
BOOL smallInsCheck[3] = { FALSE, };
BOOL subInsCheck[3] = { FALSE, };
//열차 출발/정지
BOOL trainFlag;
//열차 기본값
int trainAreaFlag;


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
	trainFlag = TRUE;

	arg1.hwnd = this->m_hWnd;
	trainAreaFlag = GetDlgItemInt(IDC_EDIT_LINE);
	if (trainAreaFlag >= 3) {
		trainAreaFlag = 1;
	}
	arg1.type = trainAreaFlag;
	if (trainCount <= 4) {
		
		m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &arg1, THREAD_PRIORITY_NORMAL, 0, 0);
		trainCount++;
	}
}

void CTrain::OnBnClickedStart()
{
	trainFlag = TRUE;

	//SetTimer(TRAIN_TIMER, 50, NULL);
}


void CTrain::OnBnClickedStop()
{
	trainFlag = FALSE;


	/*
	KillTimer(TRAIN_TIMER);
	KillTimer(WAIT_TIMER);
	//가장 최근 스레드만 정지함
	for (int i = 0; i <= trainCount; i++) {
		m_thread_move[i]->SuspendThread();
	}
	*/
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
	arraySize = (sizeof(smallRailBottomLeft) / sizeof(*smallRailBottomLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(smallRailTopLeft[i], smallRailBottomLeft[i], smallRailTopRight[i], smallRailBottomRight[i]);
		dc.Rectangle(subRailTopLeft[i], subRailBottomLeft[i], subRailTopRight[i], subRailBottomRight[i]);
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

	int flag = 1;			//열차 방향

	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치

	BOOL lineWhile = TRUE;  //반복구간 플래그용 변수
	int arraySize;

	int lineSelect;
	lineSelect = type;

	if (lineSelect == 0) {
		trainSpeed = 850;  //열차 속도
		rect = CRect(trainSpeed, 20, trainSpeed + 30, 50);
		arraySize = (sizeof(smallRailBottomLeft) / sizeof(*smallRailBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = smallRailTopLeft[i];
			threadRailTopRight[i] = smallRailTopRight[i];
			threadRailBottomLeft[i] = smallRailBottomLeft[i];
			threadRailBottomRight[i] = smallRailBottomRight[i];
		}
	}
	else if (lineSelect == 1) {
		trainSpeed = 0;  //열차 속도
		rect = CRect(trainSpeed, 20, trainSpeed + 30, 50);
		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = railTopLeft[i];
			threadRailTopRight[i] = railTopRight[i];
			threadRailBottomLeft[i] = railBottomLeft[i];
			threadRailBottomRight[i] = railBottomRight[i];
		}
	}
	else if (lineSelect == 2) {
		trainSpeed = 380;  //열차 속도
		rect = CRect(860, trainSpeed, 860 + 30, trainSpeed + 30);
		arraySize = (sizeof(subRailBottomLeft) / sizeof(*subRailBottomLeft));
		for (int i = 0; i < arraySize; i++) {
			threadRailTopLeft[i] = subRailTopLeft[i];
			threadRailTopRight[i] = subRailTopRight[i];
			threadRailBottomLeft[i] = subRailBottomLeft[i];
			threadRailBottomRight[i] = subRailBottomRight[i];
		}
	}
	CRect MainRectTest;
	CRect tmpRect;

	dc.Attach(hdc);

	int stationCount = 0; //현재 도착역
	int subStationCount = 0; //정차이후 출발역
	int safeStationCount = 1; //안전거리
	while (1) {
		if (trainFlag == TRUE) {
			Sleep(10);
			//열차 움직임 조정
			if (lineSelect == 0) {
				//안전거리용 if문
				if (insCheck[4] == FALSE && insCheck[5] == FALSE && insCheck[6] == FALSE) {
					trainSpeed -= 10;
				}
				if (stationCount <= 2) {
					//왼
					rect = CRect(trainSpeed, 70, trainSpeed + 30, 70 + 30);
					InvalidateRect(pArg->hwnd, rect, TRUE);
					trainX = trainSpeed + 30;
				}
			}
			else if (lineSelect == 1) {
				//안전거리용 if문
				if (insCheck[safeStationCount] == FALSE) {
					if (stationCount <= 6) {
						trainSpeed += 10;
					}
					else {
						trainSpeed -= 10;
					}
				}
				//열차 이동
				if (stationCount <= 4) {
					//오른
					rect = CRect(trainSpeed, 20, trainSpeed + 30, 50);
					InvalidateRect(pArg->hwnd, rect, TRUE);
					trainX = trainSpeed + 30;
				}
				else if (stationCount <= 6) {
					//아래
					rect = CRect(trainX - 30, trainSpeed, trainX, 30 + trainSpeed);
					InvalidateRect(pArg->hwnd, rect, TRUE);
					trainY = 30 + trainSpeed;
				}
				else if (stationCount <= 10) {
					//왼
					rect = CRect(trainSpeed, trainY - 30, trainSpeed + 30, trainY);
					InvalidateRect(pArg->hwnd, rect, TRUE);
					trainX = trainSpeed + 30;

				}
				else if (stationCount <= 12) {
					//위
					rect = CRect(trainX - 30, trainSpeed, trainX, 30 + trainSpeed);
					InvalidateRect(pArg->hwnd, rect, TRUE);
				}

			}
			else if (lineSelect == 2) {
				if (lineWhile == TRUE) {
					trainSpeed -= 10;
				}
				else {
					trainSpeed += 10;
				}
				if (stationCount <= 3) {
					//위
					rect = CRect(850, trainSpeed, 870 + 30, trainSpeed + 30);
					InvalidateRect(pArg->hwnd, rect, TRUE);
				}

			}
			//역 색칠하기
			CBrush brush;
			CBrush* oldBrush = dc.SelectObject(&brush);

			CRect stationRect;		//현재역 영역
			CRect subStationRect;	//이전역 영역

			dc.SelectObject(oldBrush);
			dc.Rectangle(rect);	//열차 그리기

			stationRect = CRect(threadRailTopLeft[stationCount], threadRailBottomLeft[stationCount], threadRailTopRight[stationCount], threadRailBottomRight[stationCount]);
			subStationRect = CRect(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]);


			if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0 && lineSelect == 1)
			{
				insCheck[stationCount] = TRUE;
			}

			if (lineWhile == TRUE) {
				if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0)
				{
					
					//색칠 + 무효화
					brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
					oldBrush = dc.SelectObject(&brush);
					dc.Rectangle(stationRect);
					//이전 부분과 충돌이 있을경우에만 무효화 해주기
					if (IntersectRect(tmpRect, rect, subStationRect) && stationCount >= 1) {
						InvalidateRect(pArg->hwnd, CRect(threadRailTopLeft[stationCount - 1], threadRailBottomLeft[stationCount - 1], threadRailTopRight[stationCount - 1], threadRailBottomRight[stationCount - 1]), TRUE);
					}
				}
				else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
				{
					insCheck[subStationCount] = FALSE;
					//색만 칠하기
					brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
					oldBrush = dc.SelectObject(&brush);
					dc.Rectangle(subStationRect);

				}
			}
			//반복 노선 전용
			else if (lineWhile == FALSE) {
				if (IntersectRect(tmpRect, rect, stationRect) && stationCount >= 0)
				{
					//색칠 + 무효화
					brush.CreateSolidBrush(RGB(0, 255, 0));       // 초록
					oldBrush = dc.SelectObject(&brush);
					dc.Rectangle(stationRect);
					//이전 부분과 충돌이 있을경우에만 무효화 해주기
					if (IntersectRect(tmpRect, rect, subStationRect) && stationCount >= 0) {
						InvalidateRect(pArg->hwnd, CRect(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]), TRUE);
					}

				}
				else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
				{
					brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
					oldBrush = dc.SelectObject(&brush);
					dc.Rectangle(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]);
				}
			}
			//정차, 방향 조정하기
			//2호선 외선
			if (lineSelect == 0) {
				//정차, 방향 조정
				if (stationCount <= 1 && ((100 - (stationCount * 20)) + trainSpeed) == threadRailTopRight[stationCount]) {
					subStationCount = stationCount;
					safeStationCount++;
					Sleep(500);
					stationCount++;
					//자연스러운 역 이어주기
					if (stationCount == 2) {
						threadRailTopRight[stationCount] = railTopRight[5];
					}
				}
				else if (stationCount == 2 && (50 + trainSpeed) == threadRailTopRight[stationCount]) {
					stationCount++;
					if (stationCount == 3) {
						arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));
						for (int i = 0; i < arraySize; i++) {
							threadRailTopLeft[i] = railTopLeft[i];
							threadRailTopRight[i] = railTopRight[i];
							threadRailBottomLeft[i] = railBottomLeft[i];
							threadRailBottomRight[i] = railBottomRight[i];
						}
						trainSpeed = 80;
						lineSelect = 1;
						trainX = 490;
						subStationCount = 5;
						stationCount = 6;
						safeStationCount = 7;
					}
					Sleep(500);
				}
			}
			//2호선 내선
			else if (lineSelect == 1) {
				//정차, 방향 조정
				if (stationCount <= 4 && (50 + trainSpeed) == threadRailTopRight[stationCount]) {
					//오른쪽
					flag = 1;
					subStationCount = stationCount;
					safeStationCount++;
					Sleep(1000);
					stationCount++;
					if (stationCount == 5) {
						//아래쪽
						flag = 2;
						trainSpeed = 20;
					}
				}
				else if ((stationCount >= 5 && stationCount <= 6 && (30 + trainSpeed) == threadRailBottomRight[stationCount] - 10)) {
					//아래쪽
					flag = 2;
					subStationCount = stationCount;
					safeStationCount++;
					Sleep(1000);
					stationCount++;
					if (stationCount == 7) {
						//왼쪽
						flag = 3;
						trainSpeed = trainX - 30;
					}
				}
				else if (stationCount >= 5 && stationCount <= 10 && (70 + trainSpeed) == threadRailTopRight[stationCount]) {
					//왼쪽
					flag = 3;
					subStationCount = stationCount;
					safeStationCount++;
					
					Sleep(1000);
					stationCount++;
					if (stationCount == 11) {
						//위쪽
						flag = 4;
						trainSpeed = trainY - 30;
					}
				}
				else if ((stationCount >= 11 && stationCount <= 12 && (30 + trainSpeed) == threadRailBottomRight[stationCount] - 10)) {
					//위쪽
					flag = 4;
					subStationCount = stationCount;
					safeStationCount++;
					if (safeStationCount == 13) {
						safeStationCount = 0;
					}
					Sleep(1000);
					stationCount++;
					if (stationCount == 13) {
						//오른쪽
						flag = 1;
						stationCount = 1;
						safeStationCount = 2;
						trainSpeed = 50;
						trainX = 0;
						trainY = 0;
					}
				}
			}
			//반복
			else if (lineSelect == 2) {
				//정차, 방향 조정
				if ((trainSpeed + 70) == threadRailBottomRight[stationCount]) {
					subStationCount = stationCount;
					Sleep(500);
					if (lineWhile == TRUE) {
						stationCount++;
						if (stationCount == 3) {
							subStationCount = 2;
							stationCount = 1;
							lineWhile = FALSE;
						}
					}
					else if (lineWhile == FALSE) {
						stationCount--;
						if (stationCount == -1) {
							lineWhile = TRUE;
							subStationCount = 0;
							stationCount = 1;
						}
					}
				}
			}
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
	case 0:
		dc.SelectObject(&brush);
		while (0 != DrawObject(pMain, pArg->type))
		{
			DrawObject(pMain, pArg->type);
		}
		break;
	case 1:
		dc.SelectObject(&brush);
		while (0 != DrawObject(pMain, pArg->type))
		{
			DrawObject(pMain, pArg->type);
		}
		break;
	case 2:
		dc.SelectObject(&brush);
		while (0 != DrawObject(pMain, pArg->type))
		{
			DrawObject(pMain, pArg->type);
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