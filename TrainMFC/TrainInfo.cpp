// TrainInfo.cpp: 구현 파일
//

#include "pch.h"
#include "TrainMFC.h"
#include "afxdialogex.h"
#include "TrainInfo.h"


// TrainInfo 대화 상자

IMPLEMENT_DYNAMIC(TrainInfo, CDialogEx)

TrainInfo::TrainInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_TRAININFO, pParent)
{
	*cti = { NULL };
}

TrainInfo::~TrainInfo()
{
}

void TrainInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//부모창 값을 자식창에 써주기(반복 관련)
void TrainInfo::SetFirstData(int number, int trainNumber, BOOL work, BOOL move, int count) {
	cti[number].ctiNumber = trainNumber;
	cti[number].ctiWork = work;
	cti[number].ctiMove = move;
	cti[number].ctiCount = count;
	cti[number].ctiStation = 0;
	Invalidate(TRUE);
}

//부모창 값을 자식창에 써주기(열차 운행 관련)
void TrainInfo::SetUserData(int number, BOOL work, BOOL move) {
	//달라졌을때만 바꿔주기
	if (cti[number].ctiMove != move) {
		cti[number].ctiWork = work;
		cti[number].ctiMove = move;
		Invalidate(TRUE);
	}
}

//부모창 값을 자식창에 써주기(열차 위치)
void TrainInfo::SetUserData(int number, int statation) {
	//달라졌을때만 바꿔주기
	if (cti[number].ctiStation != statation) {
		cti[number].ctiStation = statation;
		Invalidate(TRUE);
	}
}

//초기화
void TrainInfo::ChildDataDefault(int number) {
	
	cti[number].ctiNumber = 0;
	cti[number].ctiWork = FALSE;
	cti[number].ctiMove = FALSE;
	cti[number].ctiCount = 0;
	cti[number].ctiStation = 0;
	Invalidate(TRUE);
}

BEGIN_MESSAGE_MAP(TrainInfo, CDialogEx)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL TrainInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	MoveWindow(100, 100, 450, 450);

	trainInfoStatic = new CStatic();
	trainInfoStatic->Create(_T(""), WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(0, 0, 450, 450), this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void TrainInfo::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CString infoStr;
	CString tempStr = _T("\n");
	int workCheck = 0; // work check
	for (int i = 0; i < TRAIN_INFO_NUM; i++) {
		if (cti[i].ctiWork == FALSE) {
			workCheck++;
			continue;  //일 안하면 올리기
		}
		infoStr.Format(L"%d번 열차 / 운행 %d / 이동 %d / 반복 %d / 위치 %d\n\n", cti[i].ctiNumber, cti[i].ctiWork, cti[i].ctiMove, cti[i].ctiCount, cti[i].ctiStation);
		tempStr += infoStr;
		trainInfoStatic->SetWindowTextW(tempStr);
	}
	if (workCheck == TRAIN_INFO_NUM) {
		tempStr = _T("");
		trainInfoStatic->SetWindowTextW(tempStr);
	}
	

	
}

BOOL TrainInfo::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	//배경색 어둡게 바꾸기
	CRect rect;
	GetClientRect(rect);
	pDC->FillSolidRect(rect, RGB(0, 0, 0));
	return CDialogEx::OnEraseBkgnd(pDC);
}


HBRUSH TrainInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	//STATIC 컨트롤만 배경색, 글자색 변경
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(RGB(0, 0, 0));	//배경색 검정
		pDC->SetTextColor(RGB(255, 255, 255));  //글자색 흰
	}
	hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;
}
