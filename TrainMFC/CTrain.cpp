// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

// CTrain 대화 상자

IMPLEMENT_DYNAMIC(CTrain, CDialog)

CWinThread* m_thread_move[THREAD_NUM];		//열차 스레드

//열차 구역
int firstNonRevRailLeft[RAIL_NUM] = { 10, 110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400 };
int firstNonRevRailTop = 10;
int firstNonRevRailRight[RAIL_NUM] = {110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400, 1480 };
int firstNonRevRailBottom = 30;

int firstRevRailLeft[RAIL_NUM] = { 1400, 1080, 920, 820, 700, 520, 400, 280, 190, 110, 10 };
int firstRevRailTop = 40;
int firstRevRailRight[RAIL_NUM] = { 1480, 1400, 1080, 920, 820, 700, 520, 400, 280, 190, 110 };
int firstRevRailBottom = 60;

int secondRailLeft[RAIL_NUM] = { 10, 10, 110, 110, 110, 10, 10, 10, 110, 220, 220, 220, 220, 220, 220, 120 };
int secondRailTop[RAIL_NUM] = { 170, 220, 220, 270, 320, 320, 370, 420, 420, 420, 380, 320, 270, 210, 160, 160 };
int secondRailRight[RAIL_NUM] = { 110, 110, 210, 210, 210, 110, 110, 110, 220, 310, 310, 310, 310, 310, 310, 220 };
int secondRailBottom[RAIL_NUM] = { 220, 270, 270, 320, 370, 370, 420, 470, 470, 470, 420, 380, 320, 270, 210, 210 };

int thirdRailLeft[RAIL_NUM] = { 310, 420, 620, 920 };
int thirdRailTop[RAIL_NUM] = { 210, 210, 210, 210 };
int thirdRailRight[RAIL_NUM] = { 420, 620, 920, 1420 };
int thirdRailBottom[RAIL_NUM] = { 270, 270, 270, 270 };

BOOL insCheck[LINE_NUM][RAIL_NUM] = { FALSE };	//역에 열차가 있는지 확인
BOOL startInsCheck[LINE_NUM] = { FALSE };		//시작역에 열차가 있는지 확인

CString lineEditText = _T("");	//이전 선로 번호
UINT trainCount;				//열차 개수
CList<UINT, UINT&> trainNum;	//열차 번호
BOOL isCreate;					//열차 생성 가능

CString txtStr;	//Test String

UINT ThreadMoveTrain(LPVOID param);

CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	trainCount = 0;
	m_arg = { NULL };
	*m_thread_move = { NULL };
	m_hWndArg = { NULL };
}

CTrain::~CTrain()
{
}

BOOL CTrain::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	MoveWindow(100, 100, 1600, 700);	//윈도우 초기 위치, 크기 지정

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	GetDlgItem(IDC_CheckCir)->SetWindowTextW(_T("순환 가능"));
	GetDlgItem(IDC_STATIC1)->SetWindowTextW(_T("선로 번호"));
	GetDlgItem(IDC_STATIC2)->SetWindowTextW(_T("왕복 횟수"));
	GetDlgItem(IDC_STATIC)->SetWindowTextW(_T("열차 번호"));
}


BEGIN_MESSAGE_MAP(CTrain, CDialog)
	ON_BN_CLICKED(IDCREATE, &CTrain::OnBnClickedCreate)
	ON_BN_CLICKED(IDSTART, &CTrain::OnBnClickedStart)
	ON_BN_CLICKED(IDSTOP, &CTrain::OnBnClickedStop)
	ON_WM_PAINT()

END_MESSAGE_MAP()


// CTrain 메시지 처리기

void CTrain::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	//역 테두리 설정
	CPen myPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldPen = dc.SelectObject(&myPen);

	//역 만들기
	for (int i = 0; i < RAIL_NUM; i++) {
		dc.Rectangle(CRect(firstNonRevRailLeft[i], firstNonRevRailTop, firstNonRevRailRight[i], firstNonRevRailBottom));
		dc.Rectangle(CRect(firstRevRailLeft[i], firstRevRailTop, firstRevRailRight[i], firstRevRailBottom));
		dc.Rectangle(secondRailLeft[i], secondRailTop[i], secondRailRight[i], secondRailBottom[i]);
		dc.Rectangle(thirdRailLeft[i], thirdRailTop[i], thirdRailRight[i], thirdRailBottom[i]);
	}

	dc.SelectObject(oldPen);	
}

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (isCreate) return;

	CString tmpStr = _T("");

	GetDlgItemText(IDC_EDIT_LINE, tmpStr);
	UINT numLine = "" == tmpStr || "0" == tmpStr ? 1 : _wtoi(tmpStr);		//선로 번호
	GetDlgItemText(IDC_EDITCirCount, tmpStr);
	UINT numCirCount = "" == tmpStr || "0" == tmpStr ? 1 : _wtoi(tmpStr);	//순환 횟수
	BOOL checkCirEnable = ((CButton*)GetDlgItem(IDC_CheckCir))->GetCheck();	//순환 여부

	m_arg.hwnd = this->m_hWnd;
	m_arg.type = numLine;
	m_arg.cycleCount = numCirCount;
	m_arg.checkCycleEnable = checkCirEnable;
	m_arg.move = TRUE;

	if (numLine > LINE_NUM)
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Out of Line_Num=-=-=-=-=-=-=-=-=-\r\n"));
		return;
	}

	while (TRUE)
	{
		if (trainCount < THREAD_NUM)
		{
			if (NULL == trainNum.Find(trainCount))
			{
				trainNum.AddHead(trainCount);
				m_arg.id = trainCount + 1000;
				m_thread_move[trainCount] = AfxBeginThread(ThreadMoveTrain, &m_arg, THREAD_PRIORITY_NORMAL, 0, 0);
				isCreate = TRUE;
				break;
			}
			else if(NULL != trainNum.Find(trainCount))
			{
				trainCount++;
			}
		}
		else
		{
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Out of Thread_Num=-=-=-=-=-=-=-=-=-\r\n"));
			break;
		}
	}
}

void CTrain::OnBnClickedStart()
{
	int suspendCount = 0;	//일시중지 횟수
	CString editText;
	UINT trainNo;

	GetDlgItemText(IDC_EDIT_CONTROL, editText);

	if ("" == editText || "0" == editText)
	{
		for (int i = 0; i < THREAD_NUM; i++)
		{
			if (m_thread_move[i] != NULL)
			{
				do
				{
					suspendCount = m_thread_move[i]->ResumeThread();
				} while (suspendCount > 0);
			}
		}
	}
	else
	{
		trainNo = _wtoi(editText) - 1000;

		if (trainNo > (THREAD_NUM - 1) || m_thread_move[trainNo] == NULL)
		{
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Start >> NOT EXIST=-=-=-=-=-=-=-=-=-\r\n"));
			return;
		}

		do
		{
			suspendCount = m_thread_move[trainNo]->ResumeThread();
		} while (suspendCount > 0);
	}
}

void CTrain::OnBnClickedStop()
{
	CString editText;
	UINT trainNo;

	GetDlgItemText(IDC_EDIT_CONTROL, editText);

	if ("" == editText || "0" == editText)
	{
		for (int i = 0; i < THREAD_NUM; i++)
		{
			if (NULL != m_thread_move[i])
			{
				m_thread_move[i]->SuspendThread();
				
			}
		}
	}
	else
	{
		trainNo = _wtoi(editText) - 1000;

		if (trainNo > (THREAD_NUM - 1) || m_thread_move[trainNo] == NULL)
		{
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Stop >> NOT EXIST=-=-=-=-=-=-=-=-=-\r\n"));
			return;
		}

		m_thread_move[trainNo]->SuspendThread();
	}
}

void DrawFillRect(LPVOID param, CRect fillRect, int r, int g, int b)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush;
	CBrush* oldBrush;

	dc.Attach(hdc);

	//영역 칠하기
	brush.CreateSolidBrush(RGB(r, g, b));
	oldBrush = dc.SelectObject(&brush);
	dc.Rectangle(fillRect);
	dc.SelectObject(oldBrush);
	brush.DeleteObject();

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

void DrawTrainNum(LPVOID param, CRect train, UINT id)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CString trainNum;
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* oldpen;

	//열차 그리기 및 번호 표시
	dc.Attach(hdc);
	oldpen = dc.SelectObject(&pen);
	dc.Rectangle(train);
	trainNum.Format(_T("%d"), id);
	dc.DrawText(trainNum, train, DT_CENTER);
	UpdateWindow(pArg->hwnd);
	
	pen.DeleteObject();
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

UINT DrawObject(LPVOID param, int type, UINT cycleCount, BOOL checkCycleEnable, UINT id)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CRect train;			//열차 위치
	int trainSpeed = 0;		//열차 속도
	int trainX = 50;		//x기억
	int trainY = 50;		//y기억
	int trainWidth = 50;	//열차 가로 길이
	int trainHeight = 25;	//열차 세로 길이
	int flag = 1;			//열차 이동 방향		1 : Right / 2 : Down / 3 : Left / 4 : Up
	int posX = 10;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재역 좌표 번호
	int subStationCount = 0;//이전역 좌표 번호
	int lineSelect = 0;		//호선 번호

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

	int count = 0;				//선로 개수
	int cirCount = 0;			//순환 횟수(증가용)
	int cirCountText = 1;		//순환 횟수(비교용)
	BOOL cycleEnable = FALSE;	//순환 가능 여부
	CString testStr = _T("");	//Test String

	cirCountText = (cycleCount * 2);
	cycleEnable = checkCycleEnable;

	switch (type)	//선로 선택
	{
	case 1:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = firstNonRevRailLeft[i];
			TrailTop[i] = 0 == firstNonRevRailLeft[i] && 0 == firstNonRevRailRight[i] ? 0 : firstNonRevRailTop;
			TrailRight[i] = firstNonRevRailRight[i];
			TrailBottom[i] = 0 == firstNonRevRailLeft[i] && 0 == firstNonRevRailRight[i] ? 0 : firstNonRevRailBottom;
			if (TrailLeft[i] == 0 && TrailRight[i] == 0)
			{
				count = i - 1;
				break;
			}
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

			if (TrailLeft[i] == 0 && TrailRight[i] == 0)
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

			if (TrailLeft[i] == 0 && TrailRight[i] == 0)
			{
				count = i - 1;
				break;
			}
		}
		posX = 310;
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

			if (TrailLeft[i] == 0 && TrailRight[i] == 0)
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
			tmpTrailLeft[i] = firstNonRevRailLeft[i];
			tmpTrailTop[i] = firstRevRailTop;
			tmpTrailRight[i] = firstNonRevRailRight[i];
			tmpTrailBottom[i] = firstRevRailBottom;
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
			for (int i = 0; i <= count; i++)
			{
				TrailLeft[i] = tmpTrailLeft[i];
				TrailTop[i] = tmpTrailTop[i];
				TrailRight[i] = tmpTrailRight[i];
				TrailBottom[i] = tmpTrailBottom[i];
			}			
			changeDirection[2] = FALSE;
		}

		switch (flag)	//열차 이동
		{
		case 1:	//오른
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			trainX = trainSpeed + trainWidth;
			break;
		case 2:	//아래
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			trainY = trainSpeed + trainHeight;
			break;
		case 3:	//왼
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			trainX = trainSpeed + trainWidth;
			break;
		case 4:	//위
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			trainY = trainSpeed + trainHeight;
			break;
		default:
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=CTrain >> DrawObject >> Out of Flag Range=-=-=-=-=-=-=-=-=-\r\n"));
			break;
		}

		stationRect = CRect(TrailLeft[stationCount], TrailTop[stationCount], TrailRight[stationCount], TrailBottom[stationCount]);
		subStationRect = CRect(TrailLeft[subStationCount], TrailTop[subStationCount], TrailRight[subStationCount], TrailBottom[subStationCount]);
		IntersectRect(tmpRect, train, stationRect) && stationCount >= 0 ? insCheck[lineSelect][stationCount] = TRUE : NULL;
		
		if (IntersectRect(tmpRect, train, stationRect) && stationCount >= 0)
		{
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if (!insCheck[lineSelect][subStationCount] && stationCount >= 1) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			}
		}
		else if (IntersectRect(tmpRect, train, subStationRect) && subStationCount >= 0)
		{
			DrawFillRect(param, subStationRect, 255, 0, 0);
			insCheck[lineSelect][subStationCount] = FALSE;
		}

		DrawFillRect(param, stationRect, 0, 255, 0);
		DrawTrainNum(param, train, id);

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
				DrawFillRect(param, stationRect, 0, 255, 0);
				DrawTrainNum(param, train, id);
				Sleep(1000);
				isCreate = FALSE;
				stationCount++;
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed + posY) == (stationRect.bottom - 10) && moveEnable)	// 상, 하
			{
				subStationCount = stationCount;
				startInsCheck[lineSelect] = TRUE;
				DrawFillRect(param, stationRect, 0, 255, 0);
				DrawTrainNum(param, train, id);
				Sleep(1000);
				isCreate = FALSE;
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
				DrawTrainNum(param, train, id);
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
			if (!moveEnable)
			{
				DrawFillRect(param, stationRect, 255, 0, 0);
			}

			if ((trainWidth + trainSpeed + posX + 20) == stationRect.right)	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				DrawTrainNum(param, train, id);
				Sleep(1000);
				stationCount++;
			}
		}
		else // 사용하지 않는 개수 증가
		{
			stationCount++;
		}

		3 == stationCount ? startInsCheck[lineSelect] = FALSE : NULL;	// 초기 시작 충돌 방지
	}
	
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
	return id;
}

UINT ThreadMoveTrain(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	ThreadArg* pArg = (ThreadArg*)param;
	CBrush brush = RGB(255, 255, 255);
	UINT trainNo = 10000;
	DWORD dwResult = 0;
	int type = pArg->type;

	if (type > 0 && type < 5)
	{
		trainNo = DrawObject(pMain, type, pArg->cycleCount, pArg->checkCycleEnable, pArg->id);
	}
	else
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=CTrain >> ThreadMoveTrain >> Out of ThreadArg.type Range=-=-=-=-=-=-=-=-=-\r\n"));
		return 1;
	}

	if (trainNo >= 1000 && trainNo < 10000)
	{
		trainNo -= 1000;
		GetExitCodeThread(m_thread_move[trainNo]->m_hThread, &dwResult);
		trainNum.RemoveAt(trainNum.Find(trainNo));
		trainCount = 0;
	}

	if (STILL_ACTIVE != dwResult)
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=CTrain >> ThreadMoveTrain >> Thread Status != STILL_ACTIVE=-=-=-=-=-=-=-=-=-\r\n"));
		return dwResult;
	}

	return 0;
}
