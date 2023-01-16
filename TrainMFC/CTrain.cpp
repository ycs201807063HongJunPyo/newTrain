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
int soloRailTopLeft[LONG_RAIL_NUM] = { 860, 960, 1060, 1160, 1160, 1160, 1160, 1260, 1360, 1360, 1360, 1360, 1360, 1360, 1160, 960, 820, 720, 720, 620 };
int soloRailTopRight[LONG_RAIL_NUM] = { 960, 1060, 1160, 1260, 1260, 1260, 1260, 1360, 1460, 1460, 1460, 1460, 1460, 1460, 1360, 1160, 960, 820, 820, 720 };
int soloRailBottomLeft[LONG_RAIL_NUM] = { 210, 210, 210, 210, 260, 310, 360, 360, 360, 310, 260, 210, 160, 110, 110, 110, 110, 110, 60, 60 };
int soloRailBottomRight[LONG_RAIL_NUM] = { 260, 260, 260, 260 , 310, 360, 410, 410, 410, 360, 310, 260, 210, 160, 160, 160, 160, 160, 110, 110 };
//열차 구역 반복(one train NEED)
int subRailTopLeft[SUB_RAIL_NUM] = { 760, 760, 760, 760, 860, 960, 960 };
int subRailTopRight[SUB_RAIL_NUM] = { 860, 860, 860, 860, 960, 1060, 1060 };
int subRailBottomLeft[SUB_RAIL_NUM] = { 460, 410, 360, 310, 310, 310, 360 };
int subRailBottomRight[SUB_RAIL_NUM] = { 510, 460, 410, 360, 360, 360, 410 };
//열차 구역 5호선
int newRailTopLeft[LONG_RAIL_NUM] = { 10, 110, 210, 310, 410, 410, 510, 410, 510, 510, 510, 510, 510, 610, 810, 1010, 1010, 1010, 1110, 1210 };
int newRailTopRight[LONG_RAIL_NUM] = { 110, 210, 310, 410, 510, 510, 610, 510, 610, 610, 610, 610, 610, 810, 1010, 1110, 1110, 1110, 1210, 1310 };
int newRailBottomLeft[LONG_RAIL_NUM] = { 310, 310, 310, 310,310, 260, 260, 360, 360, 410, 460, 510, 560, 560, 560, 560, 510, 460, 460, 460 };
int newRailBottomRight[LONG_RAIL_NUM] = { 360, 360, 360, 360, 360, 310, 310, 410, 410, 460, 510, 560, 610, 610, 610, 610, 560, 510, 510, 510 };

BOOL insCheck[SUM_RAIL_NUM][LONG_RAIL_NUM] = { FALSE, };  //주요 열차 확인용

BOOL startInsCheck[SUM_RAIL_NUM][TRAIN_HAVE_NUM] = { FALSE, }; //각 선로에 있는 열차 체크(FALSE가 없는거, TRUE가 있는거)
BOOL joinTrain[TRAIN_HAVE_NUM] = { FALSE, };  //1번 선로 열차 진입중인지 체크
BOOL createAllCheck[SUM_RAIL_NUM] = { FALSE, };  //열차 생성 가능 체크(FALSE가 생성), startInsCheck에서 해당 선로가 깔끔한지 체크해주고 생성(1번 선로의 경우 들어오는 열차가 있는지도 체크)

//열차 생성 체크BOOL값
BOOL isCheckWhile = FALSE;

//열차 기본값
int trainAreaFlag; //열차 몇번 선택했는지(선로)
int railInTrain[SUM_RAIL_NUM]; //선로에 있는 열차 갯수
int curTrainCount; //현재 열차 갯수

//열차, 선로 정보
CString trainCountStr;  //열차 갯수 문자열
CString infoStr;  //정보 문자열

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
	ON_EN_SETFOCUS(IDC_EDIT_TRAINNUMBER, &CTrain::OnEnSetfocusEditTrainnumber)
END_MESSAGE_MAP()


// CTrain 메시지 처리기
UINT ThreadMoveTrain(LPVOID param);
//

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
	SetDlgItemInt(IDC_EDIT_TRAINNUMBER, -1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CTrain::OnBnClickedCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	trainAreaFlag = trainComboList.GetCurSel();  // 콤보박스에서 선로 받아오기
	//초기값 FALSE 재지정
	createAllCheck[trainAreaFlag] = FALSE;
	//열차가 비어있는지, 1번 선로의 경우 들어오는 열차가 있는지 체크하고 있다면 TRUE로 바꿔 생성 금지
	for (int i = 0; i < TRAIN_HAVE_NUM; i++) {
		if (startInsCheck[trainAreaFlag][i] == TRUE || (joinTrain[i] == TRUE && trainAreaFlag == 0)) {
			createAllCheck[trainAreaFlag] = TRUE;
			break;
		}
		if (i == TRAIN_HAVE_NUM) {
			createAllCheck[trainAreaFlag] = FALSE;
		}
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
			arg1[i].numberId = i + 1000;
			arg1[i].isThreadWork = TRUE;
			arg1[i].isTrainMove = TRUE;
			m_thread_move[i] = AfxBeginThread(ThreadMoveTrain, &arg1[i], THREAD_PRIORITY_NORMAL, 0, 0);
			//열차 개수 증가
			railInTrain[(trainAreaFlag)]++;
			curTrainCount++;
			//열차 갯수 텍스트 최신화
			InvalidateRect(CRect(190, 480, 350, 550), TRUE);

			CString combo;
			combo.Format(L"%d라인에 %d개 열차있음\n \n", trainAreaFlag+1, railInTrain[(trainAreaFlag)]);
			OutputDebugStringW(combo);
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
		//역 만들기(내선)
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
		//역 만들기(반복선)
		dc.Rectangle(subRailTopLeft[i], subRailBottomLeft[i], subRailTopRight[i], subRailBottomRight[i]);
	}
	//현재 열차 갯수 파악
	trainCountStr.Format(L"%d", curTrainCount);
	trainCountStr += " 현재 열차 갯수";
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
void RectPaint(LPVOID param, int trainR, int trainG, int trainB, CRect rectSize) {
	HwndArg* pArg = (HwndArg*)param;
	CDC dc;
	HDC hdc = ::GetDC(pArg->hwnd);
	CBrush brush;
	CBrush* oldBrush;
	dc.Attach(hdc);

	brush.CreateSolidBrush(RGB(trainR, trainG, trainB));
	oldBrush = dc.SelectObject(&brush);
	dc.Rectangle(rectSize);
	dc.SelectObject(oldBrush);
	brush.DeleteObject();

	dc.Detach();
	::ReleaseDC(pArg->hwnd, hdc);
}

UINT DrawObject(LPVOID param, int type, UINT numberId)
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
	trainNumberStr.Format(L"%d", (1 + lineSelect) * 10 + numberId - 1000);
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

	int stationCount = 0; //현재 도착역
	int subStationCount = 0; //정차이후 출발역
	int safeStationCount = 1; //안전거리(다음역)

	int lineRangeX;  //joinTrainToSizeX  x 기준값(알맞은 정차 위치 x 좌표)

	dc.Attach(hdc);  //hdc핸들값을 dc객체와 연결
	CRect tmpRect;  // 열차, 역 겹치는 영역
	CRect stationRect;		//현재역 영역
	CRect subStationRect;	//이전역 영역
	CRect safeStationRect;	//다음역 영역

	startInsCheck[lineSelect][(numberId - 1000)] = TRUE;
	//4,5번 같은 선로 공유
	if (lineSelect == 3) {
		startInsCheck[4][(numberId - 1000)] = TRUE;
	}
	else if (lineSelect == 4) {
		startInsCheck[3][(numberId - 1000)] = TRUE;
	}

	//열차 초기값 설정
	if (lineSelect == 0) {
		flag = 1;
		trainSpeed = 10;  //열차 속도, 초기 열차 위치
		posY = 20;
		threadRailTopLeft = railTopLeft;
		threadRailTopRight = railTopRight;
		threadRailBottomLeft = railBottomLeft;
		threadRailBottomRight = railBottomRight;

		arraySize = (sizeof(railBottomLeft) / sizeof(*railBottomLeft));  // 마지막 선로 위치 확인
	}
	else if (lineSelect == 1) {
		flag = 1;
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
	else if (lineSelect == 2) {
		flag = 4;
		trainSpeed = 480;  //열차 속도, 초기 열차 위치
		posY = 0;
		threadRailTopLeft = subRailTopLeft;
		threadRailTopRight = subRailTopRight;
		threadRailBottomLeft = subRailBottomLeft;
		threadRailBottomRight = subRailBottomRight;

		arraySize = (sizeof(subRailTopLeft) / sizeof(*subRailTopLeft));  // 마지막 선로 위치 확인
		trainX = 840;
	}
	else if (lineSelect == 3 || lineSelect == 4) {
		flag = 1;
		trainSpeed = 10;  //열차 속도, 초기 열차 위치
		posY = 320;
		//5상행을 위한 trainY 기억
		trainY = posY + trainHeight;
		threadRailTopLeft = newRailTopLeft;
		threadRailTopRight = newRailTopRight;
		threadRailBottomLeft = newRailBottomLeft;
		threadRailBottomRight = newRailBottomRight;

		lineSelect == 3 ? arraySize = (sizeof(newRailTopLeft) / sizeof(*newRailTopLeft)) : arraySize = 7;  // 마지막 선로 위치 확인
		colorR = 255;
		colorG = 0;
		colorB = 255;
	}

	while (1) {
		Sleep(20);  //기본 대기
		flagChange = 0; //플래그 조정자 초기화

		//NULL이 아닐때만, NULL 포인터 역참조 방지
		if (threadRailTopLeft != NULL && threadRailBottomLeft != NULL && threadRailTopRight != NULL && threadRailBottomRight != NULL) {
			stationRect = CRect(threadRailTopLeft[stationCount], threadRailBottomLeft[stationCount], threadRailTopRight[stationCount], threadRailBottomRight[stationCount]);
			subStationRect = CRect(threadRailTopLeft[subStationCount], threadRailBottomLeft[subStationCount], threadRailTopRight[subStationCount], threadRailBottomRight[subStationCount]);
			safeStationRect = CRect(threadRailTopLeft[safeStationCount], threadRailBottomLeft[safeStationCount], threadRailTopRight[safeStationCount], threadRailBottomRight[safeStationCount]);
		}
		else {
			break;  //만약 선로가 1개라도 NULL 이면 탈출
		}

		//열차 속도 설정(safe okay)
		if (insCheck[lineSelect][safeStationCount] == FALSE) {
			//4,5번 공용 라인 구간 앞 열차 확인
			if (stationCount <= 4 &&((lineSelect == 3 && insCheck[4][safeStationCount] == TRUE) || (lineSelect == 4 && insCheck[3][safeStationCount] == TRUE))) {
				RectPaint(param, 200, 0, 0, subStationRect);
				//열차 색상 지정
				dc.FillSolidRect(trainRect, RGB(colorR, colorG, colorB));  //흰색 없이 열차색과 텍스트만
				//열차 번호
				dc.DrawText(trainNumber, trainRect, DT_CENTER);
				//화면 업데이트로 번호 보이기
				UpdateWindow(pArg->hwnd);
				continue;
			}
			else {
				//오른(1), 아래(2) 플래그
				if (flag <= 2) {
					trainSpeed += 10;
				}
				//왼(3), 위(4) 플래그
				else if (flag <= 4) {
					trainSpeed -= 10;
				}
			}
		}
		//flag 열차 움직임 조정
		if (flag == 1) {
			//오른
			trainRect = CRect(trainSpeed, posY, trainSpeed + trainWidth, posY + trainHeight);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == 2) {
			//아래
			trainRect = CRect(trainX - trainWidth, trainSpeed, trainX, trainHeight + trainSpeed);
			trainY = trainSpeed + trainHeight;
		}
		else if (flag == 3) {
			//왼
			trainRect = CRect(trainSpeed, trainY - trainHeight, trainSpeed + trainWidth, trainY);
			trainX = trainSpeed + trainWidth;
		}
		else if (flag == 4) {
			//위
			trainRect = CRect(trainX - trainWidth, posY + trainSpeed, trainX, posY + trainHeight + trainSpeed);
			trainY = trainSpeed + trainHeight;
		}

		//반복구간이 아니고 겹쳐있다면 현재 위치에 열차 있다고 표시
		IntersectRect(tmpRect, trainRect, stationRect) && stationCount >= 0 && lineSelect != 2 ? insCheck[lineSelect][stationCount] = TRUE : NULL;
		if (IntersectRect(tmpRect, trainRect, stationRect) && stationCount >= 0)
		{
			//색칠 + 무효화
			//역 초록색(선로에 열차가 들어오거나 정차중) / 현재역
			RectPaint(param, 0, 200, 0, stationRect);
			//이전역 부분과 충돌이 있을경우에만 무효화 해주기
			if (IntersectRect(tmpRect, &trainRect, subStationRect) && (stationCount >= 1 || lineWhile == FALSE)) {
				InvalidateRect(pArg->hwnd, subStationRect, TRUE);
				insCheck[lineSelect][subStationCount] = FALSE;
			}
		}
		else if (IntersectRect(tmpRect, &trainRect, subStationRect) && subStationCount >= 0)
		{
			if (insCheck[lineSelect][safeStationCount] == FALSE) {
				//역 회색(앞에 열차가 없어서 열차가 진행할 수 있을때) / 이전역 
				RectPaint(param, 150, 150, 150, subStationRect);
			}
			else {
				//역 빨강색(앞에 열차가 있어서 열차가 진행할 수 없을때)  / 정차중
				RectPaint(param, 200, 0, 0, subStationRect);
			}
		}

		//열차 색상 지정
		dc.FillSolidRect(trainRect, RGB(colorR, colorG, colorB));  //흰색 없이 열차색과 텍스트만
		//RectPaint(param, colorR, colorG, colorB, trainRect);  //흰색 있이 열차색 + 텍스트
		//열차 번호
		dc.DrawText(trainNumber, trainRect, DT_CENTER);
		//화면 업데이트로 번호 보이기
		UpdateWindow(pArg->hwnd);
		//정차, 방향 조정하기(NULL 포인터 역참조 방지
		threadRailTopLeft != NULL ? lineRangeX = (threadRailTopRight[stationCount] - threadRailTopLeft[stationCount]) / 2 : NULL;
		///선로 이동 방향
		//생성 후 이동
		if (stationCount == 0)
		{
			if ((1 == flag || 3 == flag) && (lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				subStationCount = stationCount;
				Sleep(500);
				stationCount++;
				safeStationCount = stationCount + 1;
				//1번 선로의 경우 열차 진입 허용
				if (lineSelect == 0) {
					joinTrain[(numberId - 1000)] = FALSE;
				}
			}
			else if ((2 == flag || 4 == flag) && (trainHeight + trainSpeed) == threadRailBottomRight[stationCount] - 10)
			{
				subStationCount = stationCount;
				//반복 구간 초기값 지정
				if (lineSelect == 2) {
					trainSpeed = 480;  //열차 속도
					//위쪽
					lineWhile = TRUE;
					trainX = 840;
					flagChange = 4;
				}
				Sleep(500);
				stationCount++;
				safeStationCount = stationCount + 1;
			}
		}
		//상, 하 이동 조정자
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
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;
				if (safeStationCount < 0) {
					safeStationCount = 1;
				}
				//내선 선로 조정자
				if (lineSelect == 0) {
					if (stationCount == 12) {
						//내선 안전거리 확인
						safeStationCount = 0;
					}
					else if (stationCount == 13) {
						//내선 초기값 지정
						startInsCheck[lineSelect][(numberId - 1000)] = TRUE;
						insCheck[lineSelect][subStationCount] = FALSE;
						flag = 1;
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
				else if (lineSelect == 2) {
					if (stationCount == 7 && lineWhile == TRUE) {
						//upMove
						lineWhile = FALSE;
						subStationCount = 6;
						stationCount = 5;
						safeStationCount = 4;
						flagChange = 4;
					}
				}
				Sleep(500);  //역 도착시 정차
			}
		}
		// 좌, 우 이동 조정자
		else if (stationRect.left != subStationRect.left && stationRect.top == subStationRect.top && stationCount >= 1)
		{
			flag = stationRect.left != subStationRect.left && stationRect.left < subStationRect.left ? 3 : 1;	// T : 좌, F : 우
			if ((lineRangeX + trainSpeed) == threadRailTopRight[stationCount])
			{
				//열차 위치에 따라 자동 위치 변환 if문
				//아래로 보내기(현재역과 다음역 오른쪽이 같고, 현재역 아래보다 다음역 아래가 클때)
				if (stationRect.right == safeStationRect.right && stationRect.bottom < safeStationRect.bottom) {
					flagChange = 2;
				}
				//위로 보내기(현재역과 다음역 오른쪽이 같고, 현재역 아래가 다음역 아래보다 클때)
				else if (stationRect.right == safeStationRect.right && stationRect.bottom > safeStationRect.bottom) {
					flagChange = 4;
				}
				//역 인덱스 조정
				subStationCount = stationCount;
				lineWhile == TRUE ? stationCount++ : stationCount--;
				lineWhile == TRUE ? safeStationCount = stationCount + 1 : safeStationCount = stationCount - 1;

				//4line (하행) 열차 아래 안전거리 확인
				if (lineSelect == 3 && stationCount == 4) {
					safeStationCount = 7;
				}
				//4line (하행) 열차 아래로 밀어주기
				else if (lineSelect == 3 && stationCount == 5) {
					stationCount += 2;
					safeStationCount = stationCount + 1;
					//선로 아래로 바꾸기
					flagChange = 2;
					CString combo;
					combo.Format(L"%d :현재위치\n%d : 안전위치 \n---\n", stationCount, safeStationCount);
					OutputDebugStringW(combo);
				}

				Sleep(500);  //역 도착시 정차
			}
		}

		//열차 방향 조정값 1오른, 2아래, 3왼, 4위
		switch (flagChange)
		{
		case 1:
			flag = 1;
			posY = trainRect.top;
			trainSpeed = trainRect.left;
			break;
		case 2:
			flag = 2;
			trainSpeed = stationRect.top + 10;
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
			break;
		}
		//1번 선로 열차 들어오는거 확인
		if (lineSelect == 0) {
			if (stationCount >= 10) {
				//열차 들어오는중
				joinTrain[(numberId - 1000)] = TRUE;
			}
		}
		//열차 안전거리 지정
		if (stationCount == 2) {
			//선로에 몇번째 열차가 안전한 거리에 있는지 체크
			startInsCheck[lineSelect][(numberId - 1000)] = FALSE;
			//4,5번 같은 선로 공유
			if (lineSelect == 3 && insCheck[4][0] == FALSE) {
				startInsCheck[4][(numberId - 1000)] = FALSE;
			}
			else if (lineSelect == 4 && insCheck[3][0] == FALSE) {
				startInsCheck[3][(numberId - 1000)] = FALSE;
			}
		}
	}
	//차고지행
	insCheck[lineSelect][subStationCount] = FALSE;
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
	int errorCode = 0;
	switch ((pArg->type+1))
	{
	case 1:
		//1번 라인 최대 열차 제한
		if (railInTrain[(pArg->type)] > 5) {
			errorCode = 666;
		}
		else {
			errorCode = DrawObject(pMain, pArg->type, pArg->numberId);
		}
		break;
	case 2:
		errorCode = DrawObject(pMain, pArg->type, pArg->numberId);
		break;
	case 3:
		//3번 라인 최대 열차 제한
		if (railInTrain[pArg->type] > 1) {
			errorCode = 666;
		}
		else {
			errorCode = DrawObject(pMain, pArg->type, pArg->numberId);
		}
		break;
	case 4:
		errorCode = DrawObject(pMain, pArg->type, pArg->numberId);
		break;
	case 5:
		errorCode = DrawObject(pMain, pArg->type, pArg->numberId);
		break;
	default:
		errorCode = 666;
		OutputDebugStringW(_T("\r\n알 수 없는 값\r\n"));
		break;
	}
	if (errorCode >= 666) {
		OutputDebugStringW(_T("Thread Out\n"));
		pArg->isThreadWork = FALSE;
		pArg->isTrainMove = FALSE;
		curTrainCount--;
		railInTrain[pArg->type]--;
		if (errorCode >= 1000) {
			//열차 갯수 텍스트 최신화
			InvalidateRect(pArg->hwnd, CRect(190, 480, 350, 550), TRUE);
			::WaitForSingleObject(pMain->m_thread_move[(errorCode - 1000)], INFINITE);
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
	//에디트 박스
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_TRAINNUMBER)
	{
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 검정
		pDC->SetTextColor(RGB(255, 255, 255));  //글자색 흰
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

void CTrain::OnEnSetfocusEditTrainnumber()
{
	//에디트 박스 클릭하면 -1로 전체 출발, 정지 설정해주기
	//SetDlgItemInt(IDC_EDIT_TRAINNUMBER, -1);
	controllTrain.SetSel(0, -1, TRUE);
	//controllTrain.Clear();
}
