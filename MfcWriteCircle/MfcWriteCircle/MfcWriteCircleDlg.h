#pragma once

#include <cmath>
#include <limits>
#include <random>

// CMfcWriteCircleDlg dialog
class CMfcWriteCircleDlg : public CDialogEx
{
    // Construction
public:
    CMfcWriteCircleDlg(CWnd* pParent = nullptr);

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFCWRITECIRCLE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    // Implementation
protected:
    HICON m_hIcon;

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    CStatic m_pGetCursor;
    CStatic m_staticPic;
    int m_nRadius;
    int m_nThickness;
    static const int MAX_CIRCLE = 3;
    CPoint m_circleCenters[MAX_CIRCLE];
    int m_CntCircle;
    BOOL m_isCircle;
    
    //onPaint
    CBitmap m_canvasBitmap;
    CDC m_canvasDC;

    //드래그
    BOOL m_isDragging;
    int m_draggingIndex;
    CPoint m_dragOffset;

    //스레드
    CWinThread* m_pRandomMoveThread;
    CEvent m_eventStopThread;        
    CEvent m_eventThreadRunning; 

    void DrawCircleOnCanvas(CDC* pDC, CPoint center, double radius, int thickness, COLORREF color);
    bool CalculateCircleFromThreePoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& outCenter, double& outRadius);
    void getCurser(int centerX, int centerY);
    void getThick();
    void DrawSmallCircleOnCanvas(CDC* pDC, CPoint center, double radius, COLORREF color);
    afx_msg void OnStnClickedStaticPic();
    afx_msg void OnBnClickedButtonInit();
    afx_msg void OnBnClickedButtonRanMove();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    static UINT RandomMoveThreadFunc(LPVOID pParam);
    void OnRandomMoveThreadUpdate();
    afx_msg LRESULT OnThreadUpdate(WPARAM wParam, LPARAM lParam); 
};


#define WM_USER_THREAD_UPDATE (WM_APP + 1)