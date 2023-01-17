﻿// CTrain.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "CTrain.h"

//열차 방향용
//열차 방향 조정값 1오른, 2아래, 3왼, 4위
typedef enum
{
	RIGHT=1, BOTTOM, LEFT, TOP
}TrainFlag;
//몇 호선 열차인지 확인용(순서대로 0 ~ 4)(1~5)
typedef enum
{
	ONE, TWO, THREE, FOUR, FIVE
}RailFlag;



// CTrain 대화 상자
IMPLEMENT_DYNAMIC(CTrain, CDialog)

//열차 구역 내선
int railTopLeft[RAIL_NUM] = { 10,110,210,310,410,410,410,310,210,110,10,10,10 };
int railTopRight[RAIL_NUM] = { 110,210,310,410,510,510,510,410,310,210,110,110,110 };
int railBottomLeft[RAIL_NUM] = { 10,10,10,10,10,60,110,110,110,110,110,60,10 };
int railBottomRight[RAIL_NUM] = { 60,60,60,60,60,110,160,160,160,160,160,110,60 };
//열차 구역 1회
int soloRailTopLeft[LONG_RAIL_NUM] = { 860, 960, 1060, 1160, 1160, 1160, 1160, 1260, 1360, 1360, 1360, 1360, 1360, 1360, 1160, 960, 820, 720, 720, 620, 620 };
int soloRailTopRight[LONG_RAIL_NUM] = { 960, 1060, 1160, 1260, 1260, 1260, 1260, 1360, 1460, 1460, 1460, 1460, 1460, 1460, 1360, 1160, 960, 820, 820, 720, 720 };
int soloRailBottomLeft[LONG_RAIL_NUM] = { 210, 210, 210, 210, 260, 310, 360, 360, 360, 310, 260, 210, 160, 110, 110, 110, 110, 110, 60, 60, 110 };
int soloRailBottomRight[LONG_RAIL_NUM] = { 260, 260, 260, 260 , 310, 360, 410, 410, 410, 360, 310, 260, 210, 160, 160, 160, 160, 160, 110, 110, 160 };
//열차 구역 반복(one train NEED)
int subRailTopLeft[SUB_RAIL_NUM] = { 760, 760, 760, 760, 860, 960, 960 };
int subRailTopRight[SUB_RAIL_NUM] = { 860, 860, 860, 860, 960, 1060, 1060 };
int subRailBottomLeft[SUB_RAIL_NUM] = { 460, 410, 360, 310, 310, 310, 360 };
int subRailBottomRight[SUB_RAIL_NUM] = { 510, 460, 410, 360, 360, 360, 410 };
//열차 구역 5호선
int newRailTopLeft[LONG_RAIL_NUM] = { 10, 110, 210, 310, 410, 410, 510, 410, 510, 510, 510, 510, 510, 610, 810, 1010, 1010, 1010, 1110, 1210,1210 };
int newRailTopRight[LONG_RAIL_NUM] = { 110, 210, 310, 410, 510, 510, 610, 510, 610, 610, 610, 610, 610, 810, 1010, 1110, 1110, 1110, 1210, 1310,1310 };
int newRailBottomLeft[LONG_RAIL_NUM] = { 310, 310, 310, 310,310, 260, 260, 360, 360, 410, 460, 510, 560, 560, 560, 560, 510, 460, 460, 460,410 };
int newRailBottomRight[LONG_RAIL_NUM] = { 360, 360, 360, 360, 360, 310, 310, 410, 410, 460, 510, 560, 610, 610, 610, 610, 560, 510, 510, 510, 460 };

BOOL insCheck[SUM_RAIL_NUM][LONG_RAIL_NUM] = { FALSE, };  //주요 열차 확인용

BOOL startInsCheck[SUM_RAIL_NUM][TRAIN_HAVE_NUM] = { FALSE, }; //각 선로에 있는 열차 체크(FALSE가 없는거, TRUE가 있는거)
BOOL joinTrain[TRAIN_HAVE_NUM] = { FALSE, };  //1번 선로 열차 진입중인지 체크
BOOL createAllCheck[SUM_RAIL_NUM] = { FALSE, };  //열차 생성 가능 체크(FALSE가 생성), startInsCheck에서 해당 선로가 깔끔한지 체크해주고 생성(1번 선로의 경우 들어오는 열차가 있는지도 체크)

//열차 기본값
int trainAreaFlag; //열차 몇번 선택했는지(선로)
int railInTrain[SUM_RAIL_NUM]; //선로에 있는 열차 갯수
int curTrainCount; //현재 열차 갯수

//열차, 선로 정보
CString trainCountStr;  //열차 갯수 문자열
CString infoStr;  //정보 문자열

CWinThread* m_thread_move[TRAIN_HAVE_NUM];  //열차 쓰레드

//텍스트 렉트
CRect textRect(190, 480, 350, 550);

CTrain::CTrain(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTRAIN, pParent)
{
	*arg1 = { NULL, };
	*m_thread_move = { NULL, };
	*railInTrain = { 0, };
	curTrainCount = 0;
}

CTrain::~CTrain()
{
}

void CTrain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HINT, hintText);
	hintText.GetWindowTextW(infoStr);
	DDX_Control(pDX, IDC_COMBO_TRAINNUMBER, trainComboList);
	DDX_Control(pDX, IDC_EDIT_TRAINNUMBER, controllTrain);
}

BEGIN_MESSAGE_MAP(CTrain, CDialog)
	ON_BN_CLICKED(IDCREATE, &CTrain::OnBnClickedCreate)
	ON_BN_CLICKED(IDSTART, &CTrain::OnBnClickedStart)
	ON_BN_CLICKED(IDSTOP, &CTrain::OnBnClickedStop)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_COMBO_TRAINNUMBER, &CTrain::OnCbnSelchangeComboTrainnumber)
END_MESSAGE_MAP()


UINT ThreadMoveTrain(LPVOID param);

BOOL CTrain::OnInitDialog()
{
	CDialog::OnInitDialog();
	//화면 크기 강제 조절
	MoveWindow(100, 100, 1500, 700);
	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//ComboBox add railLine item
	trainComboList.AddString(_T("1"));
	trainComboList.AddString(_T("2"));
	trainComboList.AddString(_T("3"));
	trainComboList.AddString(_T("4"));
	trainComboList.AddString(_T("5"));
	//ComboBox default
	trainComboList.SetCurSel(0);
	//에디트박스 기본값
	SetDlgItemInt(IDC_EDIT_TRAINNUMBER, 9);  //출발, 정지 버튼 에디트 박스
	SetDlgItemInt(IDC_EDIT_TRAINCOUNT, 1);  //반복 에디트 박스
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	trainAreaFlag = trainComboList.GetCurSel();  // 콤보박스에서 선로 받아오기
	//열차 반복 횟수
	int countIndex = GetDlgItemInt(IDC_EDIT_TRAINCOUNT);
	//초기값 FALSE 재지정
	createAllCheck[trainAreaFlag] = FALSE;
	//열차가 비어있는지, 1번 선로의 경우 들어오는 열차가 있는지 체크하고 있다면 TRUE로 바꿔 생성 금지
	for (int i = 0; i < TRAIN_HAVE_NUM; i++) {
		if (startInsCheck[trainAreaFlag][i] == TRUE || (joinTrain[i] == TRUE && trainAreaFlag == 0)) {
			createAllCheck[trainAreaFlag] = TRUE;
			break;
		}
		createAllCheck[trainAreaFlag] = FALSE;
	}

	//열차 개수가 최대치를 안넘었고, 열차 생성 가능한지 체크
	if (createAllCheck[trainAreaFlag] == FALSE) {
		//for문을 통해 몇번째 열차 배치할지 체크
		for (int i = 0; i < TRAIN_HAVE_NUM; i++) {
			//해당 열차가 일하면 올라가기
			if (arg1[i].isThreadWork == TRUE) {
				continue;
			}
			//스레드에 필요한 내용들 추가
			arg1[i].hwnd = this->m_hWnd;
			arg1[i].type = trainAreaFlag;
			arg1[i].numberId = i + 10;
			arg1[i].moveCount = countIndex;
			arg1[i].isThreadWork = TRUE;
			arg1[i].isTrainMove = TRUE;
			m_thread_move[i] = AfxBeginThread(ThreadMoveTrain, &arg1[i], THREAD_PRIORITY_NORMAL, 0, 0);
			//열차 개수 증가
			railInTrain[(trainAreaFlag)]++;
			curTrainCount++;
			//열차 갯수 텍스트 최신화
			InvalidateRect(textRect, TRUE);
			break;
		}
	}


}
void CTrain::OnBnClickedStart()
{
	//열차 번호 받아오기
	int trainIndex = GetDlgItemInt(IDC_EDIT_TRAINNUMBER);
	//열차가 인덱스 범위내라면 해당 열차만 다시 운행
	if (0 <= trainIndex && TRAIN_HAVE_NUM > trainIndex && curTrainCount >= 1) {
		if (arg1[trainIndex].isThreadWork == TRUE && arg1[trainIndex].isTrainMove == FALSE) {
			arg1[trainIndex].isTrainMove = TRUE;  //한번만 동작
			m_thread_move[trainIndex]->ResumeThread();
		}
	}
	//모든 열차 다시 운행
	else {
		for (int i = 0; i < TRAIN_HAVE_NUM; i++)
		{
			//work == true -> gogo / one click
			if (arg1[i].isThreadWork == TRUE && arg1[i].isTrainMove == FALSE) {
				arg1[i].isTrainMove = TRUE;  //한번만 동작
				m_thread_move[i]->ResumeThread();
			}
		}
	}
}

void CTrain::OnBnClickedStop()
{
	//열차 번호 받아오기
	int trainIndex = GetDlgItemInt(IDC_EDIT_TRAINNUMBER);
	//열차가 인덱스 범위내라면 해당 열차만 멈추기
	if (0 <= trainIndex && TRAIN_HAVE_NUM > trainIndex && curTrainCount >= 1) {
		if (arg1[trainIndex].isThreadWork == TRUE && arg1[trainIndex].isTrainMove == TRUE) {
			arg1[trainIndex].isTrainMove = FALSE;  //한번만 동작
			m_thread_move[trainIndex]->SuspendThread();
		}
	}
	//모든 열차 멈추기
	else {
		for (int i = 0; i < TRAIN_HAVE_NUM; i++)
		{
			//work == true -> stop / one click
			if (arg1[i].isThreadWork == TRUE && arg1[i].isTrainMove == TRUE) {
				arg1[i].isTrainMove = FALSE;  //한번만 동작
				m_thread_move[i]->SuspendThread();
			}
		}
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
		//역 만들기(1번 라인(내선)
		dc.Rectangle(railTopLeft[i], railBottomLeft[i], railTopRight[i], railBottomRight[i]);
	}
	arraySize = (sizeof(soloRailTopLeft) / sizeof(*soloRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기(단일 장거리)
		dc.Rectangle(soloRailTopLeft[i], soloRailBottomLeft[i], soloRailTopRight[i], soloRailBottomRight[i]);
	}
	arraySize = (sizeof(newRailTopLeft) / sizeof(*newRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기(5호선)
		dc.Rectangle(newRailTopLeft[i], newRailBottomLeft[i], newRailTopRight[i], newRailBottomRight[i]);
	}
	arraySize = (sizeof(subRailTopLeft) / sizeof(*subRailTopLeft));
	for (int i = 0; i < arraySize; i++) {
		//역 만들기(3번 라인(반복))
		dc.Rectangle(subRailTopLeft[i], subRailBottomLeft[i], subRailTopRight[i], subRailBottomRight[i]);
	}
	//현재 열차 갯수 파악
	trainCountStr.Format(L"현재 열차 갯수 : %d", curTrainCount);
	hintText.SetWindowTextW(infoStr + trainCountStr);
	dc.SelectObject(oldPen);
}

BOOL CTrain::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 배경색 검정으로 해주기
	CRect rect;
	GetClientRect(rect);
	pDC->FillSolidRect(rect, RGB(0, 0, 0));
	return CDialog::OnEraseBkgnd(pDC);
}

//해당 렉트 색칠해주기
void RectPaint(LPVOID param, int colorR, int colorG, int colorB, CRect rectSize) {
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush;
	CBrush* oldBrush;
	dc.Attach(hdc);

	brush.CreateSolidBrush(RGB(colorR, colorG, colorB));
	oldBrush = dc.SelectObject(&brush);
	dc.Rectangle(rectSize);
	dc.SelectObject(oldBrush);
	brush.DeleteObject();

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

//해당 렉트 색칠해주기 + 열차 번호 적어주기
void RectPaint(LPVOID param, int colorR, int colorG, int colorB, CRect rectSize, CString trainNumber) {
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush;
	CBrush* oldBrush;
	CRect tmpTextRect(rectSize.left, rectSize.top + 4, rectSize.right, rectSize.bottom + 2);  //임시 렉트
	dc.Attach(hdc);

	brush.CreateSolidBrush(RGB(colorR, colorG, colorB));
	oldBrush = dc.SelectObject(&brush);
	dc.Rectangle(rectSize);
	//열차 번호
	dc.DrawText(trainNumber, tmpTextRect, DT_CENTER);
	//화면 업데이트로 번호 보이기
	UpdateWindow(pArg->hwnd);
	dc.SelectObject(oldBrush);
	brush.DeleteObject();

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

UINT DrawObject(LPVOID param, int type, UINT numberId, int moveCount)
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

	int lineSelect;  //어느 열차 선로인지 확인용
	lineSelect = type;

	//열차 관련
	CRect trainRect;  //열차 렉트
	int trainSpeed = 0;  //열차 속도
	//train color (1,3 라인 기본값)
	int colorR = 100;
	int colorG = 0;
	int colorB = 255;
	//train nicknumber
	CString trainNumberStr;
	trainNumberStr.Format(L"%d", (1 + lineSelect) * 10 + numberId - 10);
	LPCTSTR trainNumber = trainNumberStr;

	int trainX;  //x기억
	int trainY;  //y기억

	int trainWidth = 30;	//열차 가로 길이
	int trainHeight = 30;	//열차 세로 길이

	//열차 방향 조정자 1우, 2하, 3좌, 4상
	int flag = 0;			//열차 방향
	int flagChange;  //플래그 변환용

	int posX = 0;			//초기 x위치
	int posY = 0;			//초기 y위치

	int arraySize = 100;  // 선로 크기

	int stationCount = 0; //현재 도착역 카운트
	int beforeStationCount = 0; //정차이후 출발역 카운트
	int safeStationCount = 1; //안전거리(다음역) 카운트

	int count = 0;  //열차 반복 횟수(스레드 당)

	int lineRangeX;  //joinTrainToSizeX  x 기준값(알맞은 정차 위치 x 좌표)

	dc.Attach(hdc);  //hdc핸들값을 dc객체와 연결
	CRect tmpRect;  // 열차, 역 겹치는 영역
	CRect stationRect;		//현재역 영역
	CRect beforeStationRect;	//이전역 영역
	CRect safeStationRect;	//다음역 영역

	//열차 초기값 설정
	if (lineSelect == RailFlag::ONE) {
		flag = TrainFlag::RIGHT;
		trainSpeed = 20;  //열차 속도, 초기 열차 위치
		posY = 20;
		threadRailTopLeft = railTopLeft;
		threadRailTopRight = railTopRight;
		threadRailBottomLeft = railBottomLeft;
		threadRailBottomRight = railBottomRight;
		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));  // 마지막 선로 위치 확인
	}
	else if (lineSelect == RailFlag::TWO) {
		flag = TrainFlag::RIGHT;
		trainSpeed = 870;  //열차 속도, 초기 열차 위치
		posY = 220;
		threadRailTopLeft = soloRailTopLeft;
		threadRailTopRight = soloRailTopRight;
		threadRailBottomLeft = soloRailBottomLeft;
		threadRailBottomRight = soloRailBottomRight;
		arraySize = (sizeof(soloRailTopLeft) / sizeof(*soloRailTopLeft));  // 마지막 선로 위치 확인
		colorR = 150;
		colorG = 100;
		colorB = 100;
	}
	else if (lineSelect == RailFlag::THREE) {
		count = -1;
		flag = TrainFlag::TOP;
		trainSpeed = 490;  //열차 속도, 초기 열차 위치
		posY = 0;
		trainX = 840;
		threadRailTopLeft = subRailTopLeft;
		threadRailTopRight = subRailTopRight;
		threadRailBottomLeft = subRailBottomLeft;
		threadRailBottomRight = subRailBottomRight;
		arraySize = (sizeof(subRailTopLeft) / sizeof(*subRailTopLeft));  // 마지막 선로 위치 확인
		
	}
	else if (lineSelect == RailFlag::FOUR || lineSelect == RailFlag::FIVE) {
		flag = TrainFlag::RIGHT;
		trainSpeed = 20;  //열차 속도, 초기 열차 위치
		posY = 320;
		//5상행을 위한 trainY 기억
		trainY = posY + trainHeight;
		threadRailTopLeft = newRailTopLeft;
		threadRailTopRight = newRailTopRight;
		threadRailBottomLeft = newRailBottomLeft;
		threadRailBottomRight = newRailBottomRight;
		lineSelect == RailFlag::FOUR ? arraySize = (sizeof(newRailTopLeft) / sizeof(*newRailTopLeft)) : arraySize = 7;  // 마지막 선로 위치 확인
		colorR = 255;
		colorG = 0;
		colorB = 255;
	}

	//초기 반복 생성 막아주기
	startInsCheck[lineSelect][(numberId - 10)] = TRUE;
	//4,5번 같은 선로 공유
	if (lineSelect == RailFlag::FOUR) {
		startInsCheck[RailFlag::FIVE][(numberId - 10)] = TRUE;
	}
	else if (lineSelect == RailFlag::FIVE) {
		startInsCheck[RailFlag::FOUR][(numberId - 10)] = TRUE;
	}

	while (1) {
		Sleep(20);  //기본 대기
		flagChange = 0; //플래그 조정자 초기화

		//NULL이 아닐때만, NULL 포인터 역참조 방지
		if (threadRailTopLeft != NULL && threadRailBottomLeft != NULL && threadRailTopRight != NULL && threadRailBottomRight != NULL) {
			stationRect = CRect(threadRailTopLeft[stationCount], threadRailBottomLeft[stationCount], threadRailTopRight[stationCount], threadRailBottomRight[stationCount]);
			beforeStationRect = CRect(threadRailTopLeft[beforeStationCount], threadRailBottomLeft[beforeStationCount], threadRailTopRight[beforeStationCount], threadRailBottomRight[beforeStationCount]);
			safeStationRect = CRect(threadRailTopLeft[safeStationCount], threadRailBottomLeft[safeStationCount], threadRailTopRight[safeStationCount], threadRailBottomRight[safeStationCount]);
		}
		else {
			break;  //만약 선로가 1개라도 NULL 이면 탈출
		}

		//열차 속도 설정(safe okay)
		if (insCheck[lineSelect][safeStationCount] == FALSE) {
			//4,5번 공용 라인 구간 앞 열차 확인
			if (stationCount <= PUBLIC_RAIL &&((lineSelect == RailFlag::FOUR && insCheck[RailFlag::FIVE][safeStationCount] == TRUE) || (lineSelect == RailFlag::FIVE && insCheck[RailFlag::FOUR][safeStationCount] == TRUE))) {
				trainRect = CRect(trainSpeed, posY, trainSpeed + trainWidth, posY + trainHeight); //이동 못하는 열차 구역 그려주기
				RectPaint(param, 200, 0, 0, beforeStationRect);
				//열차 색상 지정
				RectPaint(param, colorR, colorG, colorB, trainRect, trainNumber);  //흰색 있이 열차색 + 텍스트
				continue;
			}
			else {
				//오른(1), 아래(2) 플래그
				if (flag <= TrainFlag::BOTTOM) {
					trainSpeed += 10;
				}
				//왼(3), 위(4) 플래그
				else if (flag <= TrainFlag::TOP) {
					trainSpeed -= 10;
				}
			}
		}
		//flag 열차 움직임 조정
		if (flag == TrainFlag::RIGHT) {
			//오른
			trainRect = CRect(trainSpeed, posY, trainSpeed + trainWidth, posY + trainHeight);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == TrainFlag::BOTTOM) {
			//아래
			trainRect = CRect(trainX - trainWidth, trainSpeed, trainX, trainHeight + trainSpeed);
			trainY = trainSpeed + trainHeight;
		}
		else if (flag == TrainFlag::LEFT) {
			//왼
			trainRect = CRect(trainSpeed, trainY - trainHeight, trainSpeed + trainWidth, trainY);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == TrainFlag::TOP) {
			//위
			trainRect = CRect(trainX - trainWidth, posY + trainSpeed, trainX, posY + trainHeight + trainSpeed);
			trainY = trainSpeed + trainHeight;
		}

		//반복구간이 아니고 겹쳐있다면 현재 위치에 열차 있다고 표시
		IntersectRect(tmpRect, trainRect, stationRect) && stationCount >= 0 && lineSelect != RailFlag::THREE ? insCheck[lineSelect][stationCount] = TRUE : NULL;
		if (IntersectRect(tmpRect, trainRect, stationRect) && stationCount >= 0)
		{
			//색칠 + 무효화
			//역 초록색(선로에 열차가 들어오거나 정차중) / 현재역
			RectPaint(param, 0, 200, 0, stationRect);
			//열차 색상 지정
			RectPaint(param, colorR, colorG, colorB, trainRect, trainNumber);
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if (IntersectRect(tmpRect, &trainRect, beforeStationRect) && (stationCount >= 1 || lineWhile == FALSE)) {
				insCheck[lineSelect][beforeStationCount] = FALSE;
				InvalidateRect(pArg->hwnd, beforeStationRect, TRUE);
				
			}
		}
		else if (IntersectRect(tmpRect, &trainRect, beforeStationRect) && beforeStationCount >= 0)
		{
			if (insCheck[lineSelect][safeStationCount] == FALSE) {
				//역 회색(앞에 열차가 없어서 열차가 진행할 수 있을때) / 이전역 
				RectPaint(param, 150, 150, 150, beforeStationRect);
				//열차 색상 지정
				RectPaint(param, colorR, colorG, colorB, trainRect, trainNumber);
			}
			else {
				//역 빨강색(앞에 열차가 있어서 열차가 진행할 수 없을때)  / 정차중
				RectPaint(param, 200, 0, 0, beforeStationRect);
				//열차 색상 지정
				RectPaint(param, colorR, colorG, colorB, trainRect, trainNumber);
			}
		}

		
		//X 방향 보정값(NULL 포인터 역참조 방지)
		threadRailTopLeft != NULL ? lineRangeX = (threadRailTopRight[stationCount] - threadRailTopLeft[stationCount]) / 2 : NULL;
		///선로 이동 방향
		//생성 후 이동
		if (stationCount == 0)
		{
			if ((TrainFlag::RIGHT == flag || TrainFlag::LEFT == flag) && (lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				beforeStationCount = stationCount;
				Sleep(500);
				stationCount++;
				safeStationCount = stationCount + 1;
				//1번 선로의 경우 열차 진입 허용
				if (lineSelect == RailFlag::ONE) {
					joinTrain[(numberId - 10)] = FALSE;
				}
			}
			else if ((TrainFlag::BOTTOM == flag || TrainFlag::TOP == flag) && (trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
			{
				beforeStationCount = stationCount;
				//반복 구간 초기값 지정
				if (lineSelect == RailFlag::THREE) {
					trainSpeed = 480;  //열차 속도
					//위쪽
					lineWhile = TRUE;
					trainX = 840;
					flagChange = TrainFlag::TOP;
					count++;
				}
				Sleep(500);
				stationCount++;
				safeStationCount = stationCount + 1;
			}
		}
		//상, 하 이동 조정자
		else if (stationRect.left == beforeStationRect.left && stationRect.top != beforeStationRect.top && stationCount >= 1)
		{
			flag = stationRect.top != beforeStationRect.top && stationRect.top > beforeStationRect.top ? TrainFlag::BOTTOM : TrainFlag::TOP;	// T : 하, F : 상
			if ((trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
			{
				//열차 위치에 따라 자동 위치 변환 if문
				//오른쪽 보내기(현재역과 다음역 아래위치가 같고, 현재역 오른쪽이 다음역 오른쪽보다 작을때)
				if (stationRect.bottom == safeStationRect.bottom && stationRect.right < safeStationRect.right) {
					flagChange = TrainFlag::RIGHT;
				}
				//왼쪽 보내기(현재역과 다음역 아래위치가 같고, 현재역 오른쪽이 다음역 오른쪽보다 클때)
				else if (stationRect.bottom == safeStationRect.bottom && stationRect.right > safeStationRect.right) {
					flagChange = TrainFlag::LEFT;
				}
				//역 인덱스 조정
				beforeStationCount = stationCount;
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;
				if (safeStationCount < 0) {
					safeStationCount = 1;
				}
				//내선 선로 조정자
				if (lineSelect == RailFlag::ONE) {
					//안전거리
					if (stationCount == (RAIL_NUM-1)) {
						safeStationCount = 0;
					}
					//내선 초기값 지정
					else if (stationCount == RAIL_NUM) {
						count++;
						startInsCheck[lineSelect][(numberId - 10)] = TRUE;
						insCheck[lineSelect][beforeStationCount] = FALSE;
						flag = TrainFlag::RIGHT;
						stationCount = 0;
						safeStationCount = 1;
						trainSpeed = 50;
						trainX = 0;
						trainY = 0;
						posY = 20;
						continue; //마지막이니까 다시 올라가기
					}
				}
				//반복 선로 조정자
				else if (lineSelect == RailFlag::THREE) {
					if (stationCount == SUB_RAIL_NUM && lineWhile == TRUE) {
						//upMove
						lineWhile = FALSE;
						beforeStationCount = 6;
						stationCount = 5;
						safeStationCount = 4;
						flagChange = TrainFlag::TOP;
					}
				}
				Sleep(500);  //역 도착시 정차
			}
		}
		// 좌, 우 이동 조정자
		else if (stationRect.left != beforeStationRect.left && stationRect.top == beforeStationRect.top && stationCount >= 1)
		{
			flag = stationRect.left != beforeStationRect.left && stationRect.left < beforeStationRect.left ? TrainFlag::LEFT : TrainFlag::RIGHT;	// T : 좌, F : 우
			if ((lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				//열차 위치에 따라 자동 위치 변환 if문
				//아래로 보내기(현재역과 다음역 오른쪽이 같고, 현재역 아래보다 다음역 아래가 클때)
				if (stationRect.right == safeStationRect.right && stationRect.bottom < safeStationRect.bottom) {
					flagChange = TrainFlag::BOTTOM;
				}
				//위로 보내기(현재역과 다음역 오른쪽이 같고, 현재역 아래가 다음역 아래보다 클때)
				else if (stationRect.right == safeStationRect.right && stationRect.bottom > safeStationRect.bottom) {
					flagChange = TrainFlag::TOP;
				}
				//역 인덱스 조정
				beforeStationCount = stationCount;
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;

				//4line (하행) 열차 아래 공용 선로 이후 안전거리 확인
				if (lineSelect == RailFlag::FOUR && stationCount == PUBLIC_RAIL) {
					safeStationCount = 7;
				}
				//4line (하행) 열차 아래로 밀어주기
				else if (lineSelect == RailFlag::FOUR && stationCount == 5) {
					stationCount += 2;
					safeStationCount = stationCount + 1;
					//선로 아래로 바꾸기
					flagChange = TrainFlag::BOTTOM;
				}
				Sleep(500);  //역 도착시 정차
			}
		}

		//열차 방향 조정값 1오른, 2아래, 3왼, 4위
		switch (flagChange)
		{
		case TrainFlag::RIGHT:
			flag = TrainFlag::RIGHT;
			posY = trainRect.top;
			trainSpeed = trainRect.left;
			break;
		case TrainFlag::BOTTOM:
			flag = TrainFlag::BOTTOM;
			trainSpeed = stationRect.top + 10;
			break;
		case TrainFlag::LEFT:
			flag = TrainFlag::LEFT;
			trainSpeed = trainX - trainWidth;
			break;
		case TrainFlag::TOP:
			flag = TrainFlag::TOP;
			trainSpeed = trainY - trainHeight;
			posY = 0;
			break;
		default:
			break;
		}
		//차고지행 확인(최대 선로 == 현재 선로)
		if (arraySize == stationCount) {
			break;
		}
		//열차 최대 반복
		if (count == moveCount) {
			//초기화시 재진입 허용
			joinTrain[(numberId - 10)] = FALSE;
			startInsCheck[lineSelect][(numberId - 10)] = FALSE;
			break;
		}
		//1번 선로 열차 들어오는거 확인
		if (lineSelect == RailFlag::ONE) {
			if (stationCount >= 10) {
				//열차 들어오는중
				joinTrain[(numberId - 10)] = TRUE;
			}
		}
		//열차 안전거리 지정
		if (stationCount == 2) {
			//선로에 몇번째 열차가 안전한 거리에 있는지 체크
			startInsCheck[lineSelect][(numberId - 10)] = FALSE;
			//4,5번 같은 선로 공유
			if (lineSelect == RailFlag::FOUR && insCheck[RailFlag::FIVE][0] == FALSE) {
				startInsCheck[RailFlag::FIVE][(numberId - 10)] = FALSE;
			}
			else if (lineSelect == RailFlag::FIVE && insCheck[RailFlag::FOUR][0] == FALSE) {
				startInsCheck[RailFlag::FOUR][(numberId - 10)] = FALSE;
			}
		}
	}
	//차고지행
	insCheck[lineSelect][beforeStationCount] = FALSE;
	InvalidateRect(pArg->hwnd, stationRect, TRUE);
	UpdateWindow(pArg->hwnd);
	//연결 해제
	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
	return numberId;
}

UINT ThreadMoveTrain(LPVOID param)
{
	CTrain* pMain = (CTrain*)param;
	ThreadArg* pArg = (ThreadArg*)param;
	DWORD dwResult;
	int code = 0;
	switch (pArg->type)
	{
	case RailFlag::ONE:
		//1번 라인 최대 열차 제한
		if (railInTrain[(pArg->type)] > 5) {
			code = 66;
		}
		else {
			code = DrawObject(pMain, pArg->type, pArg->numberId, pArg->moveCount);
		}
		break;
	case RailFlag::TWO:
		code = DrawObject(pMain, pArg->type, pArg->numberId, pArg->moveCount);
		break;
	case RailFlag::THREE:
		//3번 라인 최대 열차 제한
		if (railInTrain[pArg->type] > 1) {
			code = 66;
		}
		else {
			code = DrawObject(pMain, pArg->type, pArg->numberId, pArg->moveCount);
		}
		break;
	case RailFlag::FOUR:
		code = DrawObject(pMain, pArg->type, pArg->numberId, pArg->moveCount);
		break;
	case RailFlag::FIVE:
		code = DrawObject(pMain, pArg->type, pArg->numberId, pArg->moveCount);
		break;
	default:
		code = 66;
		OutputDebugStringW(_T("\r\nerror code 66\r\nunknow thread type"));
		break;
	}
	pArg->isThreadWork = FALSE;
	pArg->isTrainMove = FALSE;
	curTrainCount--;
	railInTrain[pArg->type]--;
	InvalidateRect(pArg->hwnd, textRect, TRUE);
	//오류 코드(생성이 안되거나, 이상한 선로)
	if (code == 66) {
		OutputDebugStringW(_T("Thread Create Error\n"));
		return 1;
	}
	//정상 종료
	else {
		code -= 10;
		if (code >= 0 && code <= 8) {
			GetExitCodeThread(m_thread_move[code]->m_hThread, &dwResult);
			if (dwResult == STILL_ACTIVE) {
				OutputDebugStringW(_T("Thread Out\n"));
			}
		}
	}
	return 0;
}

HBRUSH CTrain::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	HBRUSH hbr;

	//글자 배경, 글자 색
	//STATIC 컨트롤만 배경색, 글자색 변경
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 검정
		pDC->SetTextColor(RGB(255, 255, 255));  //글자색 흰
	}
	//에디트 박스 배경, 글자 변경
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_TRAINCOUNT || pWnd->GetDlgCtrlID() == IDC_EDIT_TRAINNUMBER)
	{
		pDC->SetBkColor(RGB(0, 0, 0));
		pDC->SetTextColor(RGB(255, 255, 255));
	}
	//콤보박스 색상
	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
	}
	hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;
}

void CTrain::OnCbnSelchangeComboTrainnumber()
{
	int selectCombo = trainComboList.GetCurSel();
	//1,3 선로면 값 그대로 두고 리드 온리 해제
	if (selectCombo == RailFlag::ONE || selectCombo == RailFlag::THREE) {
		GetDlgItem(IDC_EDIT_TRAINCOUNT)->SendMessage(EM_SETREADONLY, (WPARAM)FALSE, (LPARAM)0);
	}
	//다른 선로면 기본값 1로주고 리드 온리 설정
	else {
		SetDlgItemInt(IDC_EDIT_TRAINCOUNT, 1);
		GetDlgItem(IDC_EDIT_TRAINCOUNT)->SendMessage(EM_SETREADONLY, (WPARAM)TRUE, (LPARAM)0);
	}
}
