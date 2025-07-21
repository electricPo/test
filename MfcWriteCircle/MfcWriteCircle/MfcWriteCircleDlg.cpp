
// MfcWriteCircleDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MfcWriteCircle.h"
#include "MfcWriteCircleDlg.h"
#include "afxdialogex.h"
#include <cmath> // sqrt, pow �Լ� ���
#include <limits> // numeric_limits::epsilon() ���
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

    // ��ó ��Ʈ�Ѱ� ������ ũ���� ��Ʈ�� ���� �� DC ����
    CRect picRect;
    m_staticPic.GetClientRect(&picRect); // ��ó ��Ʈ���� Ŭ���̾�Ʈ ���� ũ��
    m_canvasBitmap.CreateCompatibleBitmap(GetDC(), picRect.Width(), picRect.Height());
    m_canvasDC.CreateCompatibleDC(GetDC());
    m_canvasDC.SelectObject(&m_canvasBitmap);

    m_nRadius = 20; // �ʱ� ������ �� ����
    m_nThickness = 2;
    UpdateData(FALSE); // ��Ʈ�ѿ� �ݿ�

    OnBnClickedButtonInit(); // �ʱ�ȭ ��ư Ŭ�� �ڵ鷯�� ȣ���Ͽ� ��Ʈ�� �� ���� �ʱ�ȭ

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
        m_staticPic.GetWindowRect(&picRect); // ��ó ��Ʈ��
        ScreenToClient(&picRect); // ���̾�α� Ŭ���̾�Ʈ ��ǥ

        // ��ó��Ʈ�ѿ� �Ѹ���
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
    //ū �� �κ�
    if (thickness <= 0)
    {
        thickness = 1; // �ּ� �β�
    }
    if (thickness > radius)
    {
        thickness = static_cast<int>(radius); // �β��� ���������� Ŭ �� ����
        if (thickness == 0) thickness = 1; // ������ 0�̸� �ּ� 1
    }

    // ���� ���� ������
    double innerRadius = radius - thickness;
    if (innerRadius < 0) innerRadius = 0; // ���δ� 0���� ���� �� ����

    //�簢�� ���
    int startX = center.x - static_cast<int>(radius);
    int startY = center.y - static_cast<int>(radius);
    int endX = center.x + static_cast<int>(radius);
    int endY = center.y + static_cast<int>(radius);

    // ��ó ��Ʈ���� Ŭ���̾�Ʈ ���� ũ�⸦ ����� �ʵ��� Ŭ����
    CRect picClientRc;
    m_staticPic.GetClientRect(&picClientRc);
    startX = max(0, startX);
    startY = max(0, startY);
    endX = min(picClientRc.Width() - 1, endX);
    endY = min(picClientRc.Height() - 1, endY);

    // �� �׸���
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

// �� ���� ������ ���� �߽ɰ� �������� ����ϴ� �Լ�
bool CMfcWriteCircleDlg::CalculateCircleFromThreePoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& outCenter, double& outRadius)
{
    // �� ���� �������� �ִ��� Ȯ�� (���� ��)
    double p12_dx = (double)(p2.x - p1.x);
    double p12_dy = (double)(p2.y - p1.y);
    double p23_dx = (double)(p3.x - p2.x);
    double p23_dy = (double)(p3.y - p2.y);

    // �� ������ ���� ���� Ȯ��
    if (std::abs(p12_dx * p23_dy - p12_dy * p23_dx) < std::numeric_limits<double>::epsilon() * 1000)
    {
        return false;
    }
    //�����̵�� ������� ū���� �߽� ���ϱ�
    double m1x = (double)(p1.x + p2.x) / 2.0; //P1�� p2�� �߰�
    double m1y = (double)(p1.y + p2.y) / 2.0;
    double m2x = (double)(p2.x + p3.x) / 2.0;
    double m2y = (double)(p2.y + p3.y) / 2.0;
    //������  �� ���� ���ϱ�
    double A1 = p12_dx;
    double B1 = p12_dy;
    double C1 = p12_dx * m1x + p12_dy * m1y;

    double A2 = p23_dx;
    double B2 = p23_dy;
    double C2 = p23_dx * m2x + p23_dy * m2y;

    double det = A1 * B2 - A2 * B1;
    //������ ������ ���� = ū ���� �߽�
    if (std::abs(det) < std::numeric_limits<double>::epsilon())
    {
        return false;
    }

    double cx = (B2 * C1 - B1 * C2) / det;
    double cy = (A1 * C2 - A2 * C1) / det;

    outCenter.x = static_cast<int>(round(cx));
    outCenter.y = static_cast<int>(round(cy));
    //ū���� ������
    outRadius = sqrt(pow(p1.x - outCenter.x, 2) + pow(p1.y - outCenter.y, 2));

    return true;
}


void CMfcWriteCircleDlg::OnStnClickedStaticPic()
{
    // TODO: Add your control notification handler code here
}


void CMfcWriteCircleDlg::OnBnClickedButtonInit()
{
    //���� ������ �ʱ�ȭ
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

    //���뿵�� x ��뿵��(��ó��Ʈ��) ����
    CRect invalidateRect;
    m_staticPic.GetWindowRect(&invalidateRect);
    ScreenToClient(&invalidateRect);
    InvalidateRect(&invalidateRect);
}


void CMfcWriteCircleDlg::OnBnClickedButtonRanMove()
{
    if (m_pRandomMoveThread != nullptr && ::WaitForSingleObject(m_pRandomMoveThread->m_hThread, 0) == WAIT_TIMEOUT)
    {
        AfxMessageBox(L"�̹� ���� �̵� �����尡 ���� ���Դϴ�.");
        return;
    }

    // 3���� ���� �׷����� ����
    if (m_CntCircle != MAX_CIRCLE)
    {
        AfxMessageBox(L"3���� ���� ���� ��� �׷����� �����̵��� ������ �� �ֽ��ϴ�.");
        return;
    }

    // �巡�� ���̶�� ���� �̵� �������� ����
    if (m_isDragging)
    {
        AfxMessageBox(L"���� ���� �巡�� ���̹Ƿ� �����̵��� ������ �� �����ϴ�.");
        return;
    }


    // ������ ����
    m_eventStopThread.ResetEvent(); 
    m_eventThreadRunning.ResetEvent(); //���� �� �ʱ�ȭ

    m_pRandomMoveThread = AfxBeginThread(RandomMoveThreadFunc, this);
    if (m_pRandomMoveThread == nullptr)
    {
        AfxMessageBox(L"���� �̵� ������ ������ �����߽��ϴ�.");
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
    // ���� �׸��� ���� �簢�� ���
    int startX = center.x - static_cast<int>(radius);
    int startY = center.y - static_cast<int>(radius);
    int endX = center.x + static_cast<int>(radius);
    int endY = center.y + static_cast<int>(radius);

    // ��ó ��Ʈ���� Ŭ���̾�Ʈ ���� ũ�⸦ ����� �ʵ��� Ŭ����
    CRect picClientRc;
    m_staticPic.GetClientRect(&picClientRc);
    startX = max(0, startX);
    startY = max(0, startY);
    endX = min(picClientRc.Width() - 1, endX);
    endY = min(picClientRc.Height() - 1, endY);

    // �ȼ� �ݺ��ϸ� �� �׸���
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
        // point�� ���̾�α� ��ǥ / ptInPic ��ó ��Ʈ�� ��� ��ǥ
        CPoint ptInPic = point - rectPic.TopLeft();
        UpdateData(TRUE);

        // ���� �߽�
        int centerX = ptInPic.x;
        int centerY = ptInPic.y;

        // ���� Ŭ�� ��ǥ ���
        getCurser(centerX, centerY);

        // �巡��
        if (m_CntCircle == MAX_CIRCLE)
        {
            for (int i = 0; i < MAX_CIRCLE; ++i)
            {
                // Ŭ�� ������ �� ���� ���� �߽� ������ �Ÿ� ���
                double distance = sqrt(pow(ptInPic.x - m_circleCenters[i].x, 2) + pow(ptInPic.y - m_circleCenters[i].y, 2));

                // �Ÿ��� ���� ���� ���������� �۰ų� ������ �ش� ���� Ŭ���� ������ ����
                if (distance <= m_nRadius)
                {
                    m_isDragging = TRUE;
                    m_draggingIndex = i;//�巡�� ���� ���� �ε��� ����
                    m_dragOffset = ptInPic - m_circleCenters[i]; // Ŭ�� ������ �� �߽� ���� ������ ���
                    return;
                }
            }
        }

        if (m_CntCircle < MAX_CIRCLE) // �� ���� ���� ��
        {
            // Ŭ���� �߽� ��ǥ�� �迭�� ����
            m_circleCenters[m_CntCircle] = CPoint(centerX, centerY);

            int currentSmallCircleRadius = m_nRadius;
            if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5; // ��ȿ�� �˻�

            DrawSmallCircleOnCanvas(&m_canvasDC, CPoint(centerX, centerY), currentSmallCircleRadius, RGB(255, 0, 0));

            m_CntCircle++; // Ŭ�� Ƚ�� ����
        }

        if (m_CntCircle == MAX_CIRCLE && m_isCircle == FALSE) //ū ��
        {
            CPoint p1 = m_circleCenters[0];
            CPoint p2 = m_circleCenters[1];
            CPoint p3 = m_circleCenters[2];
            CPoint bigCircleCenter;
            double bigCircleRadius;

            // �� ���� ������ ���� �߽ɰ� ������ ���ϱ�
            if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
            {
                // ���� ū �� �׸���
                getThick();
                DrawCircleOnCanvas(&m_canvasDC, bigCircleCenter, bigCircleRadius, m_nThickness, RGB(0, 0, 0));
            }
            else
            {
                AfxMessageBox(L"�� ���� �������� �ְų� �ʹ� ����� ���� �׸� �� �����ϴ�.");
            }
            m_isCircle = TRUE;
        }

        InvalidateRect(&rectPic); //ȭ�鰻�� -> �����ǥ��
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
        CPoint ptInPic = point - rectPic.TopLeft(); // ��ó ��Ʈ�� �� ��� ��ǥ

        // ���� ���콺 ��ġ - ����� ������
        m_circleCenters[m_draggingIndex] = ptInPic - m_dragOffset;
        // ��ó��Ʈ�� ����
        CRect clientRect;
        m_staticPic.GetClientRect(&clientRect);
        m_canvasDC.FillSolidRect(clientRect, RGB(255, 255, 255)); // ĵ���� ��ü�� ������� ����
        // ���� ��
        for (int i = 0; i < m_CntCircle; ++i)
        {
            int currentSmallCircleRadius = m_nRadius;
            if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5;
            
            DrawSmallCircleOnCanvas(&m_canvasDC, m_circleCenters[i], currentSmallCircleRadius, RGB(255, 0, 0));
        }

        // ū ��
        if (m_CntCircle == MAX_CIRCLE)
        {
            CPoint p1 = m_circleCenters[0];
            CPoint p2 = m_circleCenters[1];
            CPoint p3 = m_circleCenters[2];

            CPoint bigCircleCenter;
            double bigCircleRadius;

            if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
            {
                getThick(); // �ֽ� �β� �� ��������
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
        m_isDragging = FALSE; // �巡�� �÷��� ����
        m_draggingIndex = -1; // �ε��� �ʱ�ȭ
    }
    CDialogEx::OnLButtonUp(nFlags, point);
}

UINT CMfcWriteCircleDlg::RandomMoveThreadFunc(LPVOID pParam)
{
    CMfcWriteCircleDlg* pDlg = static_cast<CMfcWriteCircleDlg*>(pParam);
    if (pDlg == nullptr)
        return 1; // ����

    // ������ ����
    pDlg->m_eventThreadRunning.SetEvent();

    std::random_device rd;
    std::mt19937 gen(rd());

    CRect picRect;
    pDlg->m_staticPic.GetClientRect(&picRect); // ��ó ��Ʈ���� Ŭ���̾�Ʈ ���� ��������
    std::uniform_int_distribution<> distribX(0, picRect.Width());
    std::uniform_int_distribution<> distribY(0, picRect.Height());

    // 10�� �ݺ�
    for (int i = 0; i < 10; ++i)
    {
        // ������ ���� �̺�Ʈ�� �߻��ߴ��� Ȯ��
        if (::WaitForSingleObject(pDlg->m_eventStopThread.m_hObject, 0) == WAIT_OBJECT_0)
        {
            break; // �̺�Ʈ �߻� �� ������ ����
        }

        // 3���� ���� �� �߽��� ���� �̵�
        for (int j = 0; j < MAX_CIRCLE; ++j)
        {
            pDlg->m_circleCenters[j].x = distribX(gen);
            pDlg->m_circleCenters[j].y = distribY(gen);
        }

        pDlg->PostMessage(WM_USER_THREAD_UPDATE, 0, 0);

        // 0.5�� ���
        ::Sleep(500); // �и��� ����
    }

    // ������ ���Ḧ �˸�
    pDlg->m_eventThreadRunning.ResetEvent();
    return 0; // ������ ���� ����
}

void CMfcWriteCircleDlg::OnRandomMoveThreadUpdate()
{
    // ĵ���� ����� ��� �� �ٽ� �׸���
    CRect clientRect;
    m_staticPic.GetClientRect(&clientRect);
    m_canvasDC.FillSolidRect(clientRect, RGB(255, 255, 255));

    // ���� ��
    for (int i = 0; i < m_CntCircle; ++i)
    {
        int currentSmallCircleRadius = m_nRadius;
        if (currentSmallCircleRadius <= 0) currentSmallCircleRadius = 5;
        DrawSmallCircleOnCanvas(&m_canvasDC, m_circleCenters[i], currentSmallCircleRadius, RGB(255, 0, 0));
    }

    // ū ��
    if (m_CntCircle == MAX_CIRCLE)
    {
        CPoint p1 = m_circleCenters[0];
        CPoint p2 = m_circleCenters[1];
        CPoint p3 = m_circleCenters[2];

        CPoint bigCircleCenter;
        double bigCircleRadius;

        if (CalculateCircleFromThreePoints(p1, p2, p3, bigCircleCenter, bigCircleRadius))
        {
            //m_nThickness�� �̹� UpdateData(TRUE)�� �ֽ�ȭ�Ǿ� ����
            DrawCircleOnCanvas(&m_canvasDC, bigCircleCenter, bigCircleRadius, m_nThickness, RGB(0, 0, 0));
        }
    }

    // ȭ�� ���� ��û
    //InvalidateRect(&clientRect);
    //���뿵�� x ��뿵��(��ó��Ʈ��) ����
    CRect invalidateRect;
    m_staticPic.GetWindowRect(&invalidateRect);
    ScreenToClient(&invalidateRect);
    InvalidateRect(&invalidateRect);
}

// WM_USER_THREAD_UPDATE �޽��� �ڵ鷯
LRESULT CMfcWriteCircleDlg::OnThreadUpdate(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    OnRandomMoveThreadUpdate();
    return 0;
}