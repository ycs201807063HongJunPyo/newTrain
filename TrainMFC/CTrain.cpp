// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

// CTrain 대화 상자
IMPLEMENT_DYNAMIC(CTrain, CDialog)

//열차 구역 내선
int railTopLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };
int railTopRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };
int railBottomLeft[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };
int railBottomRight[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };
//열차 구역 1회
int soloRailTopLeft[SMALL_RAIL_NUM] = { 860, 960, 1060, 1160, 1160, 1160, 1160, 1260, 1360, 1360, 1360, 1360, 1360, 1360, 1260, 1060, 860, 760, 760 };
int soloRailTopRight[SMALL_RAIL_NUM] = { 960, 1060, 1160, 1260, 1260, 1260, 1260, 1360, 1460, 1460, 1460, 1460, 1460, 1460, 1360, 1260, 1060, 860, 860 };
int soloRailBottomLeft[SMALL_RAIL_NUM] = { 210, 210, 210, 210, 260, 310, 360, 360, 360, 310, 260, 210, 160, 110, 110, 110, 110, 110, 160 };
int soloRailBottomRight[SMALL_RAIL_NUM] = {  260, 260, 260, 260 , 310, 360, 410, 410, 410, 360, 310, 260, 210, 160, 160, 160, 160, 160, 210};
//열차 구역 반복(one train NEED)
int subRailTopLeft[SUB_RAIL_NUM] = { 760, 760, 760, 760, 860, 960 };
int subRailTopRight[SUB_RAIL_NUM] = { 860, 860, 860, 860, 960, 1060 };
int subRailBottomLeft[SUB_RAIL_NUM] = { 460, 410, 360, 310, 310, 310 };
int subRailBottomRight[SUB_RAIL_NUM] = { 510, 460, 410, 360, 360, 360 };
//열차 구역 5호선
int newRailTopLeft[RAIL_NUM] = { 10, 110, 210, 310, 410, 410, 510, 410, 510, 510, 510, 510, 510 };
int newRailTopRight[RAIL_NUM] = { 110, 210, 310, 410, 510, 510, 610, 510, 610, 610, 610, 610, 610 };
int newRailBottomLeft[RAIL_NUM] = { 310, 310, 310, 310,310, 360, 360, 260, 260, 410, 460, 510, 560 };
int newRailBottomRight[RAIL_NUM] = { 360, 360, 360, 360, 360, 410, 410, 310, 310, 460, 510, 560, 610 };

BOOL insCheck[SUM_RAIL_NUM][RAIL_NUM] = { FALSE, };  //주요 열차 확인용
BOOL oneTrainInsCheck = FALSE;  //열차 구역 반복(one train NEED) Check
BOOL startInsCheck[SUM_RAIL_NUM] = { FALSE, }; //시작값 체크

//열차 기본값
int trainAreaFlag;
int trainCount; //열차갯수


CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
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
	if (trainAreaFlag >= 6 || trainAreaFlag == 0) {
		trainAreaFlag = 1;
	}
	//oneTrain Check
	if (oneTrainInsCheck == TRUE && trainAreaFlag==3) {
		OutputDebugStringW(_T("dont create\n"));
		trainAreaFlag = 0;
		trainCount--;
	}
	arg1.type = trainAreaFlag;
	if (trainCount <= 4 && startInsCheck[(trainAreaFlag-1)] == FALSE ) {
		m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &arg1, THREAD_PRIORITY_NORMAL, 0, 0);
		trainCount++;
	}
}

void CTrain::OnBnClickedStart()
{
	//열차 다시 운행
	for (int i = 0; i < trainCount; i++)
	{
		m_thread_move[i]->ResumeThread();
	}

}


void CTrain::OnBnClickedStop()
{
	//열차 멈추기
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
	arraySize = (sizeof(newRailTopLeft) / sizeof(*newRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기
		dc.Rectangle(newRailTopLeft[i], newRailBottomLeft[i], newRailTopRight[i], newRailBottomRight[i]);
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

	BOOL lineWhile = TRUE;  //열차 방향 조정자(T : 정방향, F : 역방향)

	//열차 선로(포인터 -> 배열)
	int* threadRailTopLeft = 0;
	int* threadRailTopRight = 0;
	int* threadRailBottomLeft = 0;
	int* threadRailBottomRight = 0;

	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;  //열차 렉트
	int trainSpeed = 0;  //열차 속도

	int trainX;  //x기억
	int trainY;  //y기억

	int trainWidth = 30;	//열차 가로 길이
	int trainHeight = 30;	//열차 세로 길이

	//열차 방향 조정자 1우, 2하, 3좌, 4상
	int flag = 0;			//열차 방향
	int flagChange;  //플래그 변환용
	BOOL invalidateCheck;  //화면 무효화 체크용

	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치

	int arraySize = 100;  // 선로 크기

	int lineSelect;  //어느 열차 선로인지 확인용
	lineSelect = type - 1;  

	int stationCount = 0; //현재 도착역
	int subStationCount = 0; //정차이후 출발역
	int safeStationCount = 1; //안전거리

	int lineRangeX;  //trainGoToSizeX  x 기준값(알맞은 정차 위치 x 좌표)
	//열차 초기값 설정
	if (lineSelect == 0) {
		flag = 1;
		posX = 0;
		posY = 20;
		threadRailTopLeft = railTopLeft;
		threadRailTopRight = railTopRight;
		threadRailBottomLeft = railBottomLeft;
		threadRailBottomRight = railBottomRight;

		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));  // 마지막 선로 위치 확인
	}
	else if (lineSelect == 1) {
		flag = 1;
		trainSpeed = 870;  //열차 속도
		threadRailTopLeft = soloRailTopLeft;
		threadRailTopRight = soloRailTopRight;
		threadRailBottomLeft = soloRailBottomLeft;
		threadRailBottomRight = soloRailBottomRight;

		arraySize = (sizeof(soloRailTopLeft) / sizeof(*soloRailTopLeft));  // 마지막 선로 위치 확인
		posX = 870;
		posY = 220;

	}
	else if (lineSelect == 2) {
		oneTrainInsCheck = TRUE;
		flag = 4;
		trainSpeed = 480;  //열차 속도
		threadRailTopLeft = subRailTopLeft;
		threadRailTopRight = subRailTopRight;
		threadRailBottomLeft = subRailBottomLeft;
		threadRailBottomRight = subRailBottomRight;

		arraySize = (sizeof(subRailTopLeft) / sizeof(*subRailTopLeft));  // 마지막 선로 위치 확인
		posX = 840;
		trainX = posX;
		posY = 0;
	}
	else if (lineSelect == 3 || lineSelect == 4) {
		flag = 1;
		posX = 0;
		posY = 320;
		threadRailTopLeft = newRailTopLeft;
		threadRailTopRight = newRailTopRight;
		threadRailBottomLeft = newRailBottomLeft;
		threadRailBottomRight = newRailBottomRight;

		lineSelect == 3 ? arraySize = (sizeof(newRailTopLeft) / sizeof(*newRailTopLeft)) : arraySize = 9;  // 마지막 선로 위치 확인
	}

	dc.Attach(hdc);
	CRect tmpRect;
	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역
	CRect safeStationRect;	//다음역 영역

	while (1) {
		Sleep(10);  //기본 대기
		flagChange = 0; //플래그 조정자 초기화
		invalidateCheck = FALSE; //화면 무효화 초기화
		//열차 속도 설정(safe okay)
		if (insCheck[lineSelect][safeStationCount] == FALSE) {
			//오른(1), 아래(2) 플래그
			if (flag <= 2) {
				trainSpeed += 10;
			}
			//왼(3), 위(4) 플래그
			else if (flag <= 4) {
				trainSpeed -= 10;
			}
		}

		//flag 열차 움직임 조정
		if (flag == 1) {
			//오른
			rect = CRect(trainSpeed, posY, trainSpeed + trainWidth, posY + trainHeight);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == 2) {
			//아래
			rect = CRect(trainX - trainWidth, trainSpeed, trainX, trainHeight + trainSpeed);
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
		///역 색칠하기 (화면무효화)///
		CBrush brush;
		CBrush* oldBrush = dc.SelectObject(&brush);

		stationRect = CRect(threadRailTopLeft[stationCount], threadRailBottomLeft[stationCount], threadRailTopRight[stationCount], threadRailBottomRight[stationCount]);
		subStationRect = CRect(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]);
		safeStationRect = CRect(threadRailTopLeft[safeStationCount], threadRailBottomLeft[safeStationCount], threadRailTopRight[safeStationCount], threadRailBottomRight[safeStationCount]);
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
			//이전 부분과 충돌이 있을경우에만 무효화 해주기 + What lineWhile
			if (IntersectRect(tmpRect, rect, subStationRect) && !insCheck[lineSelect][subStationCount] && (stationCount >= 1 || lineWhile == FALSE)) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			}
			dc.SelectObject(oldBrush);
			brush.DeleteObject();
		}
		else if (IntersectRect(tmpRect, rect, subStationRect) && subStationCount >= 0)
		{
			insCheck[lineSelect][subStationCount] = FALSE;
			//색만 칠하기
			brush.CreateSolidBrush(RGB(255, 0, 0));       // 빨강
			oldBrush = dc.SelectObject(&brush);
			dc.Rectangle(subStationRect);
			dc.SelectObject(oldBrush);
			brush.DeleteObject();
		}

		//정차, 방향 조정하기
		lineRangeX = (threadRailTopRight[stationCount] - threadRailTopLeft[stationCount]) / 2;
		///선로 이동 방향
		//생성 후 이동
		if (stationCount == 0)
		{
			if ((1 == flag || 3 == flag) && (lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				startInsCheck[lineSelect] = TRUE;
				subStationCount = stationCount;
				Sleep(500);
				lineWhile == TRUE ? stationCount++ : stationCount--;
				safeStationCount = stationCount + 1;
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
			{
				startInsCheck[lineSelect] = TRUE;
				subStationCount = stationCount;
				//반복 구간 초기값 지정
				if (lineSelect == 2) {
					trainSpeed = 480;  //열차 속도
					//위쪽
					lineWhile = TRUE;
					trainX = posX;
					flagChange = 4;
				}
				Sleep(500);
				lineWhile == TRUE ? stationCount++ : stationCount--;
				safeStationCount = stationCount + 1;
			}
		}
		//상, 하 이동
		else if (stationRect.left == subStationRect.left && stationRect.top != subStationRect.top && stationCount >= 1)
		{
			flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? 2 : 4;	// T : 하, F : 상
			if ((trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
			{
				//열차 위치에 따라 자동 위치 변환 if문
				//오른쪽 보내기(현재역과 다음역 아래위치가 같고, 현재역 오른쪽이 다음역 오른쪽보다 작을때)
				if (stationRect.bottom == safeStationRect.bottom && stationRect.right < safeStationRect.right) {
					flagChange = 1;
				}
				//왼쪽 보내기(현재역과 다음역 아래위치가 같고, 현재역 오른쪽이 다음역 오른쪽보다 클때)
				else if (stationRect.bottom == safeStationRect.bottom && stationRect.right > safeStationRect.right) {
					flagChange = 3;
				}
				//역 인덱스 조정
				subStationCount = stationCount;
				Sleep(500);
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;
				//내선 선로 조정자
				if (lineSelect == 0) {
					if (stationCount == 13) {
						//내선 초기값 지정
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
				//5호선 선로 조정자(하행)
				else if (lineSelect == 3) {
					//오른쪽 이동할때 안전거리 미리 확보
					if (stationCount == 6) {
						safeStationCount += 2;
					}
				}
			}
		}
		// 좌, 우 이동
		else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)
		{
			flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T : 좌, F : 우
			if ((lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				//열차 위치에 따라 자동 위치 변환 if문
				//아래로 보내기((현재역과 다음역 오른쪽이 같고, 현재역 아래보다 다음역 아래가 클때)거나, 반복 구간일때)
				if (stationRect.right == safeStationRect.right && stationRect.bottom < safeStationRect.bottom) {
					flagChange = 2;
				}
				//위로 보내기(현재역과 다음역 오른쪽이 같고, 현재역 아래가 다음역 아래보다 클때)
				else if (stationRect.right == safeStationRect.right && stationRect.bottom > safeStationRect.bottom) {
					flagChange = 4;
				}
				//역 인덱스 조정
				subStationCount = stationCount;
				Sleep(500);
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;
				//내선 선로 조정자
				if (lineSelect == 0) {
					if (stationCount == 11) {
						//새로 생기는거 안전 구역
						startInsCheck[lineSelect] = TRUE;
					}
				}
				//반복 선로 조정자
				else if (lineSelect == 2) {
					if (stationCount == 6 && lineWhile == TRUE) {
						//left move
						lineWhile = FALSE;
						subStationCount = 5;
						stationCount = 4;
						safeStationCount = 3;
					}
				}
				//5호선 선로 조정자
				else if (lineSelect == 3 || lineSelect == 4) {
					//(하행) 열차 밀어주기
					if (lineSelect == 3 && stationCount == 7) {
						insCheck[lineSelect][subStationCount] = FALSE;
						InvalidateRect(pArg->hwnd, stationRect, TRUE);
						UpdateWindow(pArg->hwnd);
						stationCount += 2;
						safeStationCount = stationCount + 1;
					}
					//(상행) 열차 올려주기
					else if (lineSelect == 4 && stationCount == 5) {
						insCheck[lineSelect][subStationCount] = FALSE;
						InvalidateRect(pArg->hwnd, stationRect, TRUE);
						UpdateWindow(pArg->hwnd);
						stationCount += 2;
						safeStationCount = stationCount + 1;
						//위쪽
						flag = 4;
						trainSpeed = stationRect.top;
						posY = 0;
					}
				}
			}
		}
		//열차 방향 조정값 1오른, 2아래, 3왼, 4위
		switch (flagChange)
		{
		case 1:
			flag = 1;
			posY = rect.top;
			trainSpeed = rect.left;
			break;
		case 2:
			flag = 2;
			trainSpeed = stationRect.top;
			break;
		case 3:
			flag = 3;
			trainSpeed = trainX - trainWidth;
			break;
		case 4:
			flag = 4;
			trainSpeed = trainY - trainHeight;
			posY = 0;
			break;
		default:
			break;
		}

		//차고지행 확인(최대 선로 == 현재 선로)
		if (arraySize == stationCount) {
			invalidateCheck = TRUE;
		}

		if (invalidateCheck == TRUE) {
			trainCount--;
			insCheck[lineSelect][subStationCount] = FALSE;
			InvalidateRect(pArg->hwnd, stationRect, TRUE);
			UpdateWindow(pArg->hwnd);
			return 0;
		}

		//나는 처음에있고, 다른열차가 2번에 있는데 열차 생성하면 겹쳐나오는 오류있음
		//열차 생성시 구역에 열차 있으면 막아주기(안전 거리)
		if (stationCount == 2 && lineSelect != 0 && insCheck[lineSelect][0] == FALSE) {
			startInsCheck[lineSelect] = FALSE;
		}
		//뒤 열차 확인
		else if (stationCount == 2 && lineSelect == 0 && insCheck[lineSelect][0] == FALSE && insCheck[lineSelect][12] == FALSE && insCheck[lineSelect][11] == FALSE) {
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
	case 4:
		errorCode = DrawObject(pMain, pArg->type);
		break;
	case 5:
		errorCode = DrawObject(pMain, pArg->type);
		break;
	default:
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
		break;
	}
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);

	return 0;
}

HBRUSH CTrain::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	//STATIC 컨트롤만 배경색 변경
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 변경
	}
	//text color change
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_HINT) {
		pDC->SetTextColor(RGB(255, 255, 255));
	}
	hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;
}
