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
int railLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };
int railTop[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };
int railRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };
int railBottom[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };

int testRailLeft[RAIL_NUM] = { 520, 620, 720, 820, 820, 920, 920, 920, 820, 720, 720, 620, 520 };
int testRailTop[RAIL_NUM] = { 10,10,10,10, 60, 60, 110, 160, 160, 160, 110, 110, 110 };
int testRailRight[RAIL_NUM] = { 620, 720, 820, 920, 920, 1020, 1020, 1020, 920, 820, 820, 720, 620 };
int testRailBottom[RAIL_NUM] = { 60,60,60,60, 110, 110, 160, 210, 210, 210, 160, 160, 160 };

int TtestRailLeft[RAIL_NUM] = { 10, 10, 110, 110, 110, 10, 10, 10, 110, 220, 220, 220, 220, 220, 220, 120 };
int TtestRailTop[RAIL_NUM] = { 170, 220, 220, 270, 320, 320, 370, 420, 420, 420, 380, 320, 270, 210, 160, 160 };
int TtestRailRight[RAIL_NUM] = { 110, 110, 210, 210, 210, 110, 110, 110, 220, 310, 310, 310, 310, 310, 310, 220 };
int TtestRailBottom[RAIL_NUM] = { 220, 270, 270, 320, 370, 370, 420, 470, 470, 470, 420, 380, 320, 270, 210, 210 };

int TTtestRailLeft[RAIL_NUM] = { 310, 420, 620, 920 };
int TTtestRailTop[RAIL_NUM] = { 210, 210, 210, 210 };
int TTtestRailRight[RAIL_NUM] = { 420, 620, 920, 1420 };
int TTtestRailBottom[RAIL_NUM] = { 270, 270, 270, 270 };

int flRailLeft[RAIL_NUM] = {10, 110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400};

//역에 열차가 있는지 확인
BOOL insCheck[LINE_NUM][RAIL_NUM] = { FALSE };
BOOL testInsCheck[RAIL_NUM] = { FALSE };
BOOL startInsCheck[LINE_NUM] = { FALSE };	// 시작역에 열차가 있는지 확인
//겹치는 구역
BOOL rectResult;
CRect testRect;

UINT numLine;					//선로 번호
UINT numCirCount;				//순환 횟수
BOOL checkCirEnable;			//순환 여부
CString lineEditText = _T("");	//이전 선로 번호
int trainCount;					//열차갯수


CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	trainCount = 0;
	arg1 = { NULL };
	bmpInfo = { NULL };
	hWndArg = { NULL };
	*m_thread_move = { NULL };
}

CTrain::~CTrain()
{
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, testText1);
	DDX_Control(pDX, IDC_STATIC2, testText2);
	GetDlgItem(IDC_CheckCir)->SetWindowTextW(_T("순환 가능"));
}


BEGIN_MESSAGE_MAP(CTrain, CDialog)
	ON_BN_CLICKED(IDCREATE, &CTrain::OnBnClickedCreate)
	ON_BN_CLICKED(IDSTART, &CTrain::OnBnClickedStart)
	ON_BN_CLICKED(IDSTOP, &CTrain::OnBnClickedStop)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//ON_WM_CTLCOLOR()

	ON_EN_CHANGE(IDC_EDIT_LINE, &CTrain::OnEnChangeEditLine)
END_MESSAGE_MAP()


// CTrain 메시지 처리기
UINT ThreadMoveTrain(LPVOID param);

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString tmpStr = _T("");

	GetDlgItemText(IDC_EDIT_LINE, tmpStr);
	numLine = _wtoi(tmpStr);
	GetDlgItemText(IDC_EDITCirCount, tmpStr);
	numCirCount = "" == tmpStr || "0" == tmpStr ? 1 : _wtoi(tmpStr);
	checkCirEnable = ((CButton*)GetDlgItem(IDC_CheckCir))->GetCheck();
	
	arg1.hwnd = this->m_hWnd;
	arg1.type = numLine;
	if (trainCount <= (THREAD_NUM - 1)) {
		arg1.id = trainCount + 10000;
		m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &arg1, THREAD_PRIORITY_NORMAL, 0, 0);
		//GetDlgItem(IDCREATE)->EnableWindow(FALSE);
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
	// 비트맵 리소스 로딩
	CBitmap bmp;
	CBitmap* pOldBmp = NULL;

	//맵 만들기
	MemDC.SelectObject(pOldBmp);
	//역 테두리 설정
	CPen myPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = dc.SelectObject(&myPen);
	int arraySize = (sizeof(flRailLeft) / sizeof(*flRailLeft)) / 2;

	//역 만들기
	for (int i = 0; i < arraySize; i++)
	{
		dc.Rectangle(CRect(flRailLeft[i], 10, flRailLeft[i + 1], 30));
		dc.Rectangle(CRect(flRailLeft[i], 40, flRailLeft[i + 1], 60));
	}
	for (int i = 0; i < RAIL_NUM; i++) {
		
		dc.Rectangle(TtestRailLeft[i], TtestRailTop[i], TtestRailRight[i], TtestRailBottom[i]);
		dc.Rectangle(TTtestRailLeft[i], TTtestRailTop[i], TTtestRailRight[i], TTtestRailBottom[i]);
	}
	
	dc.SelectObject(oldPen);

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

UINT DrawObject(LPVOID param, int type, UINT id)
{
	CTrain* pMain = (CTrain*)param;
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect rect;				//열차
	int trainSpeed = 0;		//열차 속도
	int trainX = 50;		//x기억
	int trainY = 50;		//y기억
	int trainWidth = 30;	//열차 가로 길이
	int trainHeight = 10;	//열차 세로 길이
	int flag = 1;			//열차 이동 방향		1 : Right / 2 : Down / 3 : Left / 4 : Up
	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재 도착역
	int subStationCount = 0;//정차이후 출발역
	int lineSelect = 0;		//선로 번호저장

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
	BOOL cirEnable = FALSE;	//순환 가능 여부
	int arraySize = 0;

	cirCountText = (numCirCount * 2);
	cirEnable = checkCirEnable;

	switch (type)	//선로 선택
	{
	case 2:
		lineSelect = type - 1;
		arraySize = (sizeof(flRailLeft) / sizeof(*flRailLeft)) / 2;
		for (int i = 0; i < arraySize; i++)
		{
			TrailLeft[i] = flRailLeft[i];
			TrailTop[i] = 10;
			TrailRight[i] = flRailLeft[i + 1];
			TrailBottom[i] = 30;
		}
		break;
	case 3:
		lineSelect = type - 1;
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = TtestRailLeft[i];
			TrailTop[i] = TtestRailTop[i];
			TrailRight[i] = TtestRailRight[i];
			TrailBottom[i] = TtestRailBottom[i];

			if (TrailLeft[i] == 0 && TrailTop[i] == 0 && TrailRight[i] == 0 && TrailBottom[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posY = 160;
		break;
	case 4:
		lineSelect = type - 1;
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = TTtestRailLeft[i];
			TrailTop[i] = TTtestRailTop[i];
			TrailRight[i] = TTtestRailRight[i];
			TrailBottom[i] = TTtestRailBottom[i];

			if (TrailLeft[i] == 0 && TrailTop[i] == 0 && TrailRight[i] == 0 && TrailBottom[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posX = 300;
		posY = 210;
		break;
	case 5:
		lineSelect = type - 1;
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = TtestRailLeft[i];
			TrailTop[i] = TtestRailTop[i];
			TrailRight[i] = TtestRailRight[i];
			TrailBottom[i] = TtestRailBottom[i];

			if (i > 13)
			{
				TrailLeft[i] = TTtestRailLeft[i - 14];
				TrailTop[i] = TTtestRailTop[i - 14];
				TrailRight[i] = TTtestRailRight[i - 14];
				TrailBottom[i] = TTtestRailBottom[i - 14];
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

	for (int i = 0; i <= count; i++)
	{
		tmpTrailLeft[i] = TrailLeft[i];
		tmpTrailTop[i] = TrailTop[i];
		tmpTrailRight[i] = TrailRight[i];
		tmpTrailBottom[i] = TrailBottom[i];
	}

	rect = CRect(trainSpeed + posX, trainY - trainHeight, trainSpeed + trainWidth, trainY);	//열차 초기 위치 설정
	dc.Attach(hdc);

	while (TRUE) {
		Sleep(10);	//이동 딜레이

		if (cirCount == cirCountText)	// 순환 종료 및 반환
		{
			InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			return id;
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
			for (int i = 0; i <= count; i++)
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
			moveEnable ? trainSpeed += 10 : Sleep(10);
			rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 2:	//아래
			moveEnable ? trainSpeed += 10 : Sleep(10);
			rect = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainY = trainSpeed + trainHeight;
			break;
		case 3:	//왼
			moveEnable ? trainSpeed -= 10 : Sleep(10);
			rect = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			InvalidateRect(pArg->hwnd, rect, TRUE);
			trainX = trainSpeed + trainWidth;
			break;
		case 4:	//위
			moveEnable ? trainSpeed -= 10 : Sleep(10);
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
				cirCount++;
			}
			else
			{
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				return id;
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

	return 1;
}

UINT TestDrawObject(LPVOID param)	//그리기 및 이동 테스트용
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

	cirCountText = (numCirCount * 2);
	cirEnable = checkCirEnable;
	
	int arraySize = (sizeof(flRailLeft) / sizeof(*flRailLeft)) / 2;
	for (int i = 0; i < arraySize; i++)
	{
		TrailLeft[i] = flRailLeft[i];
		TrailTop[i] = 10;
		TrailRight[i] = flRailLeft[i + 1];
		TrailBottom[i] = 30;
	}

	for (int i = 0; i < arraySize; i++)
	{
		tmpTrailLeft[i] = flRailLeft[i];
		tmpTrailTop[i] = 40;
		tmpTrailRight[i] = flRailLeft[i + 1];
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
			return 2;
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
				//return id;
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

	return 1;
}

UINT ThreadMoveTrain(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	ThreadArg* pArg = (ThreadArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush = RGB(255, 255, 255);
	int errorCode = 10000;
	DWORD dwResult;

	dc.Attach(hdc);
	dc.SelectObject(&brush);
	switch (pArg->type)
	{
	case 2:
		errorCode = DrawObject(pMain, pArg->type, pArg->id);
		break;
	case 3:
		errorCode = DrawObject(pMain, pArg->type, pArg->id);
		break;
	case 4:
		errorCode = DrawObject(pMain, pArg->type, pArg->id);
		break;
	case 5:
		errorCode = DrawObject(pMain, pArg->type, pArg->id);
		break;
	case 6:
		errorCode = DrawObject(pMain, pArg->type, pArg->id);
		break;
	case 100:
		errorCode = 100;
		dc.SelectObject(&brush);
		TestDrawObject(pMain);
		break;
	default:
		errorCode = 2;
		OutputDebugStringW(_T("\r\nCTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range\r\n"));
		break;
	}

	if (errorCode >= 10000)
	{
		::GetExitCodeThread(pMain->m_thread_move[(errorCode - 10000)], &dwResult);
		trainCount--;
	}
		
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);

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

void CTrain::OnEnChangeEditLine()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString tmpStr = _T("NULL");

	GetDlgItemTextW(IDC_EDIT_LINE, tmpStr);

	if (tmpStr != "" && lineEditText != tmpStr)
	{
		GetDlgItem(IDCREATE)->EnableWindow(TRUE);
		//startInsCheck = FALSE;
		lineEditText = tmpStr;
	}
		
}
