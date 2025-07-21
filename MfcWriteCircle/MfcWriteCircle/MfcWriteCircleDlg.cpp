
// MfcWriteCircleDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MfcWriteCircle.h"
#include "MfcWriteCircleDlg.h"
#include "afxdialogex.h"
#include <cmath> // sqrt, pow 함수 사용
#include <limits> // numeric_limits::epsilon() 사용
#include <random> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);   

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CMfcWriteCircleDlg dialog



CMfcWriteCircleDlg::CMfcWriteCircleDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_MFCWRITECIRCLE_DIALOG, pParent)
    , m_nRadius(0) 
    , m_CntCircle(0) 
    , m_isCircle(FALSE) 
    , m_isDragging(FALSE) 
    , m_draggingIndex(-1) 
    , m_pRandomMoveThread(nullptr)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfcWriteCircleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_X_AND_Y_PTR, m_pGetCursor);
    DDX_Control(pDX, IDC_STATIC_PIC, m_staticPic);
    DDX_Text(pDX, IDC_EDIT_RADIUS, m_nRadius);
    DDX_Text(pDX, IDC_EDIT_THICK, m_nThickness);
}

BEGIN_MESSAGE_MAP(CMfcWriteCircleDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()


    ON_STN_CLICKED(IDC_STATIC_PIC, &CMfcWriteCircleDlg::OnStnClickedStaticPic)
    ON_BN_CLICKED(IDC_BUTTON_INIT, &CMfcWriteCircleDlg::OnBnClickedButtonInit)
    ON_BN_CLICKED(IDC_BUTTON_RAN_MOVE, &CMfcWriteCircleDlg::OnBnClickedButtonRanMove)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_MESSAGE(WM_USER_THREAD_UPDATE, &CMfcWriteCircleDlg::OnThreadUpdate)
END_MESSAGE_MAP()


// CMfcWriteCircleDlg message handlers

BOOL CMfcWriteCircleDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here

    // 픽처 컨트롤과 동일한 크기의 비트맵 생성 및 DC 연결
    CRect picRect;
    m_staticPic.GetClientRect(&picRect); // 픽처 컨트롤의 클라이언트 영역 크기
    m_canvasBitmap.CreateCompatibleBitmap(GetDC(), picRect.Width(), picRect.Height());
    m_canvasDC.CreateCompatibleDC(GetDC());
    m_canvasDC.SelectObject(&m_canvasBitmap);

    m_nRadius = 20; // 초기 반지름 값 설정
    m_nThickness = 2;
    UpdateData(FALSE); // 컨트롤에 반영

    OnBnClickedButtonInit(); // 초기화 버튼 클릭 핸들러를 호출하여 비트맵 및 상태 초기화

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMfcWriteCircleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMfcWriteCircleDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CPaintDC dc(this);
        CRect picRect;
        m_staticPic.GetWindowRect(&picRect); // 픽처 컨트롤
        ScreenToClient(&picRect); // 다이얼로그 클라이언트 좌표

        // 픽처컨트롤에 뿌리기
        dc.BitBlt(picRect.left, picRect.top, picRect.Width(), picRect.Height(), &m_canvasDC, 0, 0, SRCCOPY);

    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMfcWriteCircleDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CMfcWriteCircleDlg::DrawCircleOnCanvas(CDC* pDC, CPoint center, double radius, int thickness, COLORREF color)
{
    //큰 원 부분
    if (thickness <= 0)
    {
        thickness = 1; // 최소 두께
    }
    if (thickness > radius)
    {
        thickness = static_cast<int>(radius); // 두께가 반지름보다 클 수 없음
        if (thickness == 0) thickness = 1; // 반지름 0이면 최소 1
    }

    // 원의 내부 반지름
    double innerRadius = radius - thickness;
    if (innerRadius < 0) innerRadius = 0; // 내부는 0보다 작을 수 없음

    //사각형 경계
    int startX = center.x - static_cast<int>(radius);
    int startY = center.y - static_cast<int>(radius);
    int endX = center.x + static_cast<int>(radius);
    int endY = center.y + static_cast<int>(radius);

    // 픽처 컨트롤의 클라이언트 영역 크기를 벗어나지 않도록 클리핑
    CRect picClientRc;
    m_staticPic.GetClientRect(&picClientRc);
    startX = max(0, startX);
    startY = max(0, startY);
    endX = min(picClientRc.Width() - 1, endX);
    endY = min(picClientRc.Height() - 1, endY);

    // 원 그리기
    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            double distance = sqrt(pow(x - center.x, 2) + pow(y - center.y, 2));

            if (distance >= innerRadius && distance <= radius)
            {
                pDC->SetPixel(x, y, color);
            }
        }
    }
}

// 세 점을 지나는 원의 중심과 반지름을 계산하는 함수
bool CMfcWriteCircleDlg::CalculateCircleFromThreePoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& outCenter, double& outRadius)
{
    // 세 점이 일직선에 있는지 확인 (기울기 비교)
    double p12_dx = (double)(p2.x - p1.x);
    double p12_dy = (double)(p2.y - p1.y);
    double p23_dx = (double)(p3.x - p2.x);
    double p23_dy = (double)(p3.y - p2.y);

    // 두 구간의 평행 여부 확인
    if (std::abs(p12_dx * p23_dy - p12_dy * p23_dx) < std::numeric_limits<double>::epsilon() * 1000)
    {
        return false;
    }
    //수직이등분 방식으로 큰원의 중심 구하기
    double m1x = (double)(p1.x + p2.x) / 2.0; //P1과 p2의 중간
    double m1y = (double)(p1.y + p2.y) / 2.0;
    double m2x = (double)(p2.x + p3.x) / 2.0;
    double m2y = (double)(p2.y + p3.y) / 2.0;
    //수직의  두 선분 구하기
    double A1 = p12_dx;
    double B1 = p12_dy;
    double C1 = p12_dx * m1x + p12_dy * m1y;

    double A2 = p23_dx;
    double B2 = p23_dy;
    double C2 = p23_dx * m2x + p23_dy * m2y;

    double det = A1 * B2 - A2 * B1;
    //선분이 만나는 지점 = 큰 원의 중심
    if (std::abs(det) < std::numeric_limits<double>::epsilon())
    {
        return false;
    }

    double cx = (B2 * C1 - B1 * C2) / det;
    double cy = (A1 * C2 - A2 * C1) / det;

    outCenter.x = static_cast<int>(round(cx));
    outCenter.y = static_cast<int>(round(cy));
    //큰원의 반지름
    outRadius = sqrt(pow(p1.x - outCenter.x, 2) + pow(p1.y - outCenter.y, 2));

    return true;
}


void CMfcWriteCircleDlg::OnStnClickedStaticPic()
{
    // TODO: Add your control notification handler code here
}


void CMfcWriteCircleDlg::OnBnClickedButtonInit()
{
    //기존 스레드 초기화
    if (m_pRandomMoveThread != nullptr)
    {
        m_eventStopThread.SetEvent(); 
        ::WaitForSingleObject(m_pRandomMoveThread->m_hThread, INFINITE); 
        m_pRandomMoveThread = nullptr; 
        m_eventStopThread.ResetEvent(); 
        m_eventThreadRunning.ResetEvent();
    }

    CRect picRect;
    m_staticPic.GetClientRect(&picRect);
    m_canvasDC.FillSolidRect(picRect, RGB(255, 255, 255));

    m_isCircle = FALSE;
    m_CntCircle = 0;
    m_isDragging = FALSE; 
    m_draggingIndex = -1;

    m_pGetCursor.SetWindowText(L"X: -, Y: -");

    //절대영역 x 상대영역(픽처컨트롤) 갱신
    CRect invalidateRect;
    m_staticPic.GetWindowRect(&invalidateRect);
    ScreenToClient(&invalidateRect);
    InvalidateRect(&invalidateRect);
}


void CMfcWriteCircleDlg::OnBnClickedButtonRanMove()
{
    if (m_pRandomMoveThread != nullptr && ::WaitForSingleObject(m_pRandomMoveThread->m_hThread, 0) == WAIT_TIMEOUT)
    {
        AfxMessageBox(L"이미 랜덤 이동 스레드가 실행 중입니다.");
        return;
    }

    // 3개의 원이 그려져야 실행
    if (m_CntCircle != MAX_CIRCLE)
    {
        AfxMessageBox(L"3개의 작은 원이 모두 그려져야 랜덤이동을 시작할 수 있습니다.");
        return;
    }

    // 드래그 중이라면 랜덤 이동 시작하지 않음
    if (m_isDragging)
    {
        AfxMessageBox(L"현재 원을 드래그 중이므로 랜덤이동을 시작할 수 없습니다.");
        return;
    }


    // 스레드 시작
    m_eventStopThread.ResetEvent(); 
    m_eventThreadRunning.ResetEvent(); //시작 전 초기화

    m_pRandomMoveThread = AfxBeginThread(RandomMoveThreadFunc, this);
    if (m_pRandomMoveThread == nullptr)
    {
        AfxMessageBox(L"랜덤 이동 스레드 생성에 실패했습니다.");
        return;
    }
    m_pRandomMoveThread->m_bAutoDelete = TRUE;
}

void CMfcWriteCircleDlg::getCurser(int centerX, int centerY)
{
    CString strCoords;
    strCoords.Format(L"%d, %d", centerX, centerY);
    m_pGetCursor.SetWindowText(strCoords);
}
void CMfcWriteCircleDlg::getThick()
{
    m_nThickness  = GetDlgItemInt(IDC_EDIT_THICK);
}
void CMfcWriteCircleDlg::DrawSmallCircleOnCanvas(CDC* pDC, CPoint center, double radius, COLORREF color)
{
    // 원을 그리기 위한 사각형 경계
    int startX = center.x - static_cast<int>(radius);
    int startY = center.y - static_cast<int>(radius);
    int endX = center.x + static_cast<int>(radius);
    int endY = center.y + static_cast<int>(radius);

    // 픽처 컨트롤의 클라이언트 영역 크기를 벗어나지 않도록 클리핑
    CRect picClientRc;
    m_staticPic.GetClientRect(&picClientRc);
    startX = max(0, startX);
    startY = max(0, startY);
    endX = min(picClientRc.Width() - 1, endX);
    endY = min(picClientRc.Height() - 1, endY);

    // 픽셀 반복하며 원 그리기
    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            double distance = sqrt(pow(x - center.x, 2) + pow(y - center.y, 2));
            if (distance <= radius)
            {
                pDC->SetPixel(x, y, color);
            }
        }
    }
}
void CMfcWriteCircleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect rectPic;
    m_staticPic.GetWindowRect(&rectPic);
    ScreenToClient(&rectPic);

    if (rectPic.PtInRect(point))
    {
        // point는 다이얼로그 좌표 / ptInPic 픽처 컨트롤 상대 좌표
        CPoint ptInPic = point - rectPic.TopLeft();
        UpdateData(TRUE);

        // 원의 중심
        int centerX = ptInPic.x;
        int centerY = ptInPic.y;

        // 현재 클릭 좌표 출력
        getCurser(centerX, centerY);

        // 드래그
        if (m_CntCircle == MAX_CIRCLE)
        {
            for (int i = 0; i < MAX_CIRCLE; ++i)
            {
                // 클릭 지점과 각 작은 원의 중심 사이의 거리 계산
                double distance = sqrt(pow(ptInPic.x - m_circleCenters[i].x, 2) + pow(ptInPic.y - m_circleCenters[i].y, 2));

                // 거리가 작은 원의 반지름보다 작거나 같으면 해당 원을 클릭한 것으로 간주
                if (distance <= m_nRadius)
                {
                    m_isDragging = TRUE;
                    m_draggingIndex = i;//드래그 중인 원의 인덱스 저장
                    m_dragOffset = ptInPic - m_circleCenters[i]; // 클릭 지점과 원 중심 간의 오프셋 계산
                    return;
                }
            }
        }

        if (m_CntCircle < MAX_CIRCLE) // 세 개의 작은 원
        {
            // 클릭된 중심 좌표를 배열에 저장
            m_circleCenters[m_CntCircle] = CPoint(centerX, centerY);

            int currentSmallCircleRadius = m_nRadius;
            if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5; // 유효성 검사

            DrawSmallCircleOnCanvas(&m_canvasDC, CPoint(centerX, centerY), currentSmallCircleRadius, RGB(255, 0, 0));

            m_CntCircle++; // 클릭 횟수 증가
        }

        if (m_CntCircle == MAX_CIRCLE && m_isCircle == FALSE) //큰 원
        {
            CPoint p1 = m_circleCenters[0];
            CPoint p2 = m_circleCenters[1];
            CPoint p3 = m_circleCenters[2];
            CPoint bigCircleCenter;
            double bigCircleRadius;

            // 세 점을 지나는 원의 중심과 반지름 구하기
            if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
            {
                // 계산된 큰 원 그리기
                getThick();
                DrawCircleOnCanvas(&m_canvasDC, bigCircleCenter, bigCircleRadius, m_nThickness, RGB(0, 0, 0));
            }
            else
            {
                AfxMessageBox(L"세 점이 일직선상에 있거나 너무 가까워 원을 그릴 수 없습니다.");
            }
            m_isCircle = TRUE;
        }

        InvalidateRect(&rectPic); //화면갱신 -> 상대좌표로
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void CMfcWriteCircleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_isDragging)
    {
        CRect rectPic;
        m_staticPic.GetWindowRect(&rectPic);
        ScreenToClient(&rectPic);
        CPoint ptInPic = point - rectPic.TopLeft(); // 픽처 컨트롤 내 상대 좌표

        // 현재 마우스 위치 - 저장된 오프셋
        m_circleCenters[m_draggingIndex] = ptInPic - m_dragOffset;
        // 픽처컨트롤 영역
        CRect clientRect;
        m_staticPic.GetClientRect(&clientRect);
        m_canvasDC.FillSolidRect(clientRect, RGB(255, 255, 255)); // 캔버스 전체를 흰색으로 지움
        // 작은 원
        for (int i = 0; i < m_CntCircle; ++i)
        {
            int currentSmallCircleRadius = m_nRadius;
            if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5;
            
            DrawSmallCircleOnCanvas(&m_canvasDC, m_circleCenters[i], currentSmallCircleRadius, RGB(255, 0, 0));
        }

        // 큰 원
        if (m_CntCircle == MAX_CIRCLE)
        {
            CPoint p1 = m_circleCenters[0];
            CPoint p2 = m_circleCenters[1];
            CPoint p3 = m_circleCenters[2];

            CPoint bigCircleCenter;
            double bigCircleRadius;

            if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
            {
                getThick(); // 최신 두께 값 가져오기
                DrawCircleOnCanvas(&m_canvasDC, bigCircleCenter, bigCircleRadius, m_nThickness, RGB(0, 0, 0));
            }
        }

        InvalidateRect(&rectPic);
    }

    CDialogEx::OnMouseMove(nFlags, point);
}


void CAboutDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CDialogEx::OnLButtonUp(nFlags, point);
}


void CMfcWriteCircleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_isDragging)
    {
        m_isDragging = FALSE; // 드래그 플래그 해제
        m_draggingIndex = -1; // 인덱스 초기화
    }
    CDialogEx::OnLButtonUp(nFlags, point);
}

UINT CMfcWriteCircleDlg::RandomMoveThreadFunc(LPVOID pParam)
{
    CMfcWriteCircleDlg* pDlg = static_cast<CMfcWriteCircleDlg*>(pParam);
    if (pDlg == nullptr)
        return 1; // 오류

    // 스레드 시작
    pDlg->m_eventThreadRunning.SetEvent();

    std::random_device rd;
    std::mt19937 gen(rd());

    CRect picRect;
    pDlg->m_staticPic.GetClientRect(&picRect); // 픽처 컨트롤의 클라이언트 영역 가져오기
    std::uniform_int_distribution<> distribX(0, picRect.Width());
    std::uniform_int_distribution<> distribY(0, picRect.Height());

    // 10번 반복
    for (int i = 0; i < 10; ++i)
    {
        // 스레드 종료 이벤트가 발생했는지 확인
        if (::WaitForSingleObject(pDlg->m_eventStopThread.m_hObject, 0) == WAIT_OBJECT_0)
        {
            break; // 이벤트 발생 시 스레드 종료
        }

        // 3개의 작은 원 중심점 랜덤 이동
        for (int j = 0; j < MAX_CIRCLE; ++j)
        {
            pDlg->m_circleCenters[j].x = distribX(gen);
            pDlg->m_circleCenters[j].y = distribY(gen);
        }

        pDlg->PostMessage(WM_USER_THREAD_UPDATE, 0, 0);

        // 0.5초 대기
        ::Sleep(500); // 밀리초 단위
    }

    // 스레드 종료를 알림
    pDlg->m_eventThreadRunning.ResetEvent();
    return 0; // 스레드 정상 종료
}

void CMfcWriteCircleDlg::OnRandomMoveThreadUpdate()
{
    // 캔버스 지우고 모든 원 다시 그리기
    CRect clientRect;
    m_staticPic.GetClientRect(&clientRect);
    m_canvasDC.FillSolidRect(clientRect, RGB(255, 255, 255));

    // 작은 원
    for (int i = 0; i < m_CntCircle; ++i)
    {
        int currentSmallCircleRadius = m_nRadius;
        if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5;
        DrawSmallCircleOnCanvas(&m_canvasDC, m_circleCenters[i], currentSmallCircleRadius, RGB(255, 0, 0));
    }

    // 큰 원
    if (m_CntCircle == MAX_CIRCLE)
    {
        CPoint p1 = m_circleCenters[0];
        CPoint p2 = m_circleCenters[1];
        CPoint p3 = m_circleCenters[2];

        CPoint bigCircleCenter;
        double bigCircleRadius;

        if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
        {
            //m_nThickness는 이미 UpdateData(TRUE)로 최신화되어 있음
            DrawCircleOnCanvas(&m_canvasDC, bigCircleCenter, bigCircleRadius, m_nThickness, RGB(0, 0, 0));
        }
    }

    // 화면 갱신 요청
    //InvalidateRect(&clientRect);
    //절대영역 x 상대영역(픽처컨트롤) 갱신
    CRect invalidateRect;
    m_staticPic.GetWindowRect(&invalidateRect);
    ScreenToClient(&invalidateRect);
    InvalidateRect(&invalidateRect);
}

// WM_USER_THREAD_UPDATE 메시지 핸들러
LRESULT CMfcWriteCircleDlg::OnThreadUpdate(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    OnRandomMoveThreadUpdate();
    return 0;
}