// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

// CTrain 대화 상자

IMPLEMENT_DYNAMIC(CTrain, CDialog)

enum TrainDirection
{
	Left = 1,
	Up = 2,
	Right = 3,
	Down = 4
};

CWinThread* m_thread_move[THREAD_NUM];		//열차 스레드

//열차 구역
int firstRailLeft[RAIL_NUM] = { 10, 110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400 };
int firstRailTop[3] = { 10, 40, 70 };
int firstRailRight[RAIL_NUM] = {110, 190, 280, 400, 520, 700, 820, 920, 1080, 1400, 1480 };
int firstRailBottom[3] = { 30, 60, 90 };

int secondRailLeft[RAIL_NUM] = { 10, 10, 110, 110, 110, 10, 10, 10, 110, 220, 220, 220, 220, 220, 220, 120 };
int secondRailTop[RAIL_NUM] = { 170, 220, 220, 270, 320, 320, 370, 420, 420, 420, 380, 320, 270, 210, 160, 160 };
int secondRailRight[RAIL_NUM] = { 110, 110, 210, 210, 210, 110, 110, 110, 220, 310, 310, 310, 310, 310, 310, 220 };
int secondRailBottom[RAIL_NUM] = { 220, 270, 270, 320, 370, 370, 420, 470, 470, 470, 420, 380, 320, 270, 210, 210 };

int thirdRailLeft[RAIL_NUM] = {310, 310, 420, 620, 920 };
int thirdRailTop[RAIL_NUM] = {150, 210, 210, 210, 210 };
int thirdRailRight[RAIL_NUM] = {420, 420, 620, 920, 1420 };
int thirdRailBottom[RAIL_NUM] = {210, 270, 270, 270, 270 };

BOOL insCheck[LINE_NUM][RAIL_NUM] = { FALSE };	//역에 열차가 있는지 확인
BOOL startInsCheck[LINE_NUM] = { FALSE };		//시작역에 열차가 있는지 확인

CString lineEditText = _T("");		//이전 선로 번호
CList<UINT, UINT&> firstTrainNum;	//1라인 열차 리스트
UINT firstTrainCount;				//1라인 열차 개수
CList<UINT, UINT&> trainNum;		//열차 번호 리스트
UINT trainCount;					//총 열차 개수
BOOL isCreate;						//열차 생성 가능

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
		dc.Rectangle(firstRailLeft[i], firstRailTop[0], firstRailRight[i], firstRailBottom[0]);
		dc.Rectangle(firstRailLeft[i], firstRailTop[1], firstRailRight[i], firstRailBottom[1]);
		dc.Rectangle(firstRailLeft[i], firstRailTop[2], firstRailRight[i], firstRailBottom[2]);
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
	else if (startInsCheck[(numLine - 1)])
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Exist Train on Select LineNum=-=-=-=-=-=-=-=-=-\r\n"));
		return;
	}
	else if (2 == numLine && (startInsCheck[(numLine - 1)] || startInsCheck[3]))
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Exist Train on Select LineNum=-=-=-=-=-=-=-=-=-\r\n"));
		return;
	}
	else if (3 == numLine && startInsCheck[(numLine - 1)])
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Exist Train on Select LineNum=-=-=-=-=-=-=-=-=-\r\n"));
		return;
	}
	else if (4 == numLine && (startInsCheck[(numLine - 1)] || startInsCheck[1]))
	{
		OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Create >> Exist Train on Select LineNum=-=-=-=-=-=-=-=-=-\r\n"));
		return;
	}

	while (TRUE)
	{
		if (1 != numLine) break;

		if (firstTrainNum.GetCount() < 3)
		{
			if (NULL == firstTrainNum.Find(firstTrainCount))
			{
				firstTrainNum.AddHead(firstTrainCount);
				break;
			}
			else if (NULL != firstTrainNum.Find(firstTrainCount))
			{
				firstTrainCount++;
			}
		}
		else
		{
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Max Train Number=-=-=-=-=-=-=-=-=-\r\n"));
			return;
		}
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
				1 == numLine ? NULL : isCreate = TRUE;
				break;
			}
			else if(NULL != trainNum.Find(trainCount))
			{
				trainCount++;
			}
		}
		else
		{
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=Max Train Number=-=-=-=-=-=-=-=-=-\r\n"));
			return;
		}
	}
}

void CTrain::OnBnClickedStart()
{
	int suspendCount = 0;	//일시중지 횟수
	CString editText;		//열차번호(텍스트)
	UINT trainNo;			//열차번호(UINT)

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
	CString editText;	//열차번호(텍스트)
	UINT trainNo;		//열차번호(UINT)

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

void DrawTrainNum(LPVOID param, CRect train, BOOL reverse, UINT id)
{
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CString trainNum;	//열차번호
	CBrush brush = RGB(128, 128, 128);
	CBrush* oldBrush;

	//열차 그리기 및 번호 표시
	dc.Attach(hdc);

	if (reverse)
	{
		oldBrush = dc.SelectObject(&brush);
		dc.Rectangle(train);
		dc.SelectObject(oldBrush);
	}
	else
	{
		dc.Rectangle(train);
	}
	
	trainNum.Format(_T("%d"), id);
	dc.DrawText(trainNum, train, DT_CENTER);
	UpdateWindow(pArg->hwnd);
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
	int trainHeight = 20;	//열차 세로 길이
	int flag = 3;			//열차 이동 방향		1 : Left / 2 : Up / 3 : Right / 4 : Down
	int posX = 10;			//초기 x위치
	int posY = 0;			//초기 y위치
	int stationCount = 0;	//현재역 좌표 번호
	int subStationCount = 0;//이전역 좌표 번호
	int lineSelect = 0;		//노선 번호

	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역

	CRect interRect;		//곂치는 영역 저장
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

	UINT firstLineNum = firstTrainCount;	//자신의 선로 번호(1라인 전용)

	cirCountText = (cycleCount * 2);
	cycleEnable = checkCycleEnable;

	switch (type)	//선로 선택
	{
	case 1:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = firstRailLeft[i];
			TrailTop[i] = firstRailTop[firstTrainCount];
			TrailRight[i] = firstRailRight[i];
			TrailBottom[i] = firstRailBottom[firstTrainCount];
			if (0 == TrailLeft[i] && 0 == TrailRight[i])
			{
				count = i - 1;
				break;
			}
		}
		trainY = 30;
		posY = firstRailTop[firstTrainCount] - 10;
		break;
	case 2:
		for (int i = 0; i < RAIL_NUM; i++)
		{
			TrailLeft[i] = secondRailLeft[i];
			TrailTop[i] = secondRailTop[i];
			TrailRight[i] = secondRailRight[i];
			TrailBottom[i] = secondRailBottom[i];

			if (0 == TrailLeft[i] && 0 == TrailRight[i])
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

			if (0 == TrailLeft[i] && 0 == TrailRight[i])
			{
				count = i - 1;
				break;
			}
		}
		posX = 330;
		posY = 150;
		flag = TrainDirection::Down;
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
				TrailLeft[i] = thirdRailLeft[i - 13];
				TrailTop[i] = thirdRailTop[i - 13];
				TrailRight[i] = thirdRailRight[i - 13];
				TrailBottom[i] = thirdRailBottom[i - 13];
			}

			if (0 == TrailLeft[i] && 0 == TrailRight[i])
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
	
	for (int i = 0; i <= count; i++)
	{
		tmpTrailLeft[i] = TrailLeft[i];
		tmpTrailTop[i] = TrailTop[i];
		tmpTrailRight[i] = TrailRight[i];
		tmpTrailBottom[i] = TrailBottom[i];
	}
	
	dc.Attach(hdc);
	
	while (TRUE) {
		Sleep(10);	//이동 딜레이

		if (cirCount == cirCountText)	// 순환 종료 및 반환
		{
			InvalidateRect(pArg->hwnd, subStationRect, TRUE);
			insCheck[lineSelect][subStationCount] = FALSE;
			if (0 == lineSelect)
			{
				firstTrainNum.RemoveAt(firstTrainNum.Find(firstLineNum));
			}
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
		case TrainDirection::Left:
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			trainX = trainSpeed + trainWidth;
			break;
		case TrainDirection::Up:
			moveEnable ? trainSpeed -= 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			trainY = trainSpeed + trainHeight;
			break;
		case TrainDirection::Right:
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainSpeed + posX, trainY - trainHeight + posY, trainSpeed + trainWidth + posX, trainY + posY);
			trainX = trainSpeed + trainWidth;
			break;
		case TrainDirection::Down:
			moveEnable ? trainSpeed += 5 : Sleep(10);
			train = CRect(trainX - trainWidth + posX, trainSpeed + posY, trainX + posX, trainSpeed + trainHeight + posY);
			trainY = trainSpeed + trainHeight;
			break;
		default:
			OutputDebugStringW(_T("\r\n-=-=-=-=-=-=-=-=-=CTrain >> DrawObject >> Out of Flag Range=-=-=-=-=-=-=-=-=-\r\n"));
			break;
		}

		stationRect = CRect(TrailLeft[stationCount], TrailTop[stationCount], TrailRight[stationCount], TrailBottom[stationCount]);
		subStationRect = CRect(TrailLeft[subStationCount], TrailTop[subStationCount], TrailRight[subStationCount], TrailBottom[subStationCount]);
		
		if (IntersectRect(interRect, train, stationRect) && stationCount >= 0 && moveEnable)
		{
			insCheck[lineSelect][stationCount] = TRUE;
			insCheck[lineSelect][subStationCount] = FALSE;
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if ((!insCheck[lineSelect][subStationCount] && stationCount >= 1) || 0 == lineSelect)
			{
				InvalidateRect(pArg->hwnd, subStationRect, FALSE);
			}
		}
		else if ((IntersectRect(interRect, train, subStationRect) && subStationCount >= 0) || 0 == lineSelect && moveEnable)
		{
			DrawFillRect(param, subStationRect, 255, 0, 0);
		}

		DrawFillRect(param, stationRect, 0, 255, 0);
		DrawTrainNum(param, train, isReverse, id);
		testStr.Format(_T("%d >> %d %d\n"), id, insCheck[lineSelect][stationCount], insCheck[lineSelect][stationCount + 1]);
		OutputDebugStringW(testStr);

		if (stationCount == (count + 1))	// 이동 종료
		{
			if (cycleEnable)
			{
				insCheck[lineSelect][subStationCount] = FALSE;
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
				insCheck[lineSelect][subStationCount] = FALSE;
				if (0 == lineSelect)
				{
					firstTrainNum.RemoveAt(firstTrainNum.Find(firstLineNum));
				}
				break;
			}
			
		}
		else if (stationCount == 0 && !isReverse)	// 생성 후 이동
		{
			moveEnable = insCheck[lineSelect][stationCount + 1] && 0 != lineSelect ? FALSE : TRUE;
			if ((1 == flag || 3 == flag) && (trainWidth + trainSpeed + posX) == (stationRect.right - 20) && moveEnable)	// 좌, 우
			{
				subStationCount = stationCount;
				if (0 != lineSelect)
				{
					startInsCheck[lineSelect] = TRUE;
				}
				DrawFillRect(param, stationRect, 0, 255, 0);
				DrawTrainNum(param, train, isReverse, id);
				Sleep(1000);
				isCreate = FALSE;
				stationCount++;
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed + posY) == (stationRect.bottom - 10) && moveEnable)	// 상, 하
			{
				subStationCount = stationCount;
				if (0 != lineSelect)
				{
					startInsCheck[lineSelect] = TRUE;
				}
				DrawFillRect(param, stationRect, 0, 255, 0);
				DrawTrainNum(param, train, isReverse, id);
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

			flag = stationRect.top != subStationRect.top && stationRect.top > subStationRect.top ? TrainDirection::Down : TrainDirection::Up;	// T(4) : 하, F(2) : 상
			moveEnable = insCheck[lineSelect][stationCount + 1] ? FALSE : TRUE;

			if ((trainHeight + trainSpeed + posY) == (stationRect.bottom - 10))	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				DrawTrainNum(param, train, isReverse, id);
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

			flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? TrainDirection::Left : TrainDirection::Right;	// T(1) : 좌, F(3) : 우
			moveEnable = insCheck[lineSelect][stationCount + 1] && 0 != lineSelect ? FALSE : TRUE;

			if (!moveEnable)
			{
				DrawFillRect(param, stationRect, 255, 0, 0);
			}

			if ((trainWidth + trainSpeed + posX + 20) == stationRect.right)	// 일정거리 이동 후 정지
			{
				subStationCount = stationCount;
				moveEnable = FALSE;
				DrawFillRect(param, stationRect, 255, 0, 0);
				DrawTrainNum(param, train, isReverse, id);
				Sleep(1000);
				stationCount++;
			}
		}
		else // 사용하지 않는 개수 증가
		{
			stationCount++;
		}

		if (3 == stationCount && !insCheck[lineSelect][subStationCount])	// 초기 시작 충돌 방지
		{
			startInsCheck[lineSelect] = FALSE;
		}
	}
	
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
	pArg = NULL;
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
		trainNum.RemoveAt(trainNum.Find(trainNo));
		trainCount = 0;
		firstTrainCount = 0;
	}
	
	return 0;
}
