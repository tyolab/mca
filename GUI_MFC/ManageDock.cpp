#include "stdafx.h"
#include "ManageDock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CManageDock, CDockablePane)

BEGIN_MESSAGE_MAP(CManageDock, CDockablePane)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()

CManageDock::CManageDock()
{
}


CManageDock::~CManageDock()
{
}

//CConfigView * CManageDock::getConfigViewCamera()
//{
//	CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
//	CConfigView* pView = DYNAMIC_DOWNCAST(CConfigView, pWnd);
//	return pView;
//}
//
//CConfigView * CManageDock::getConfigViewCamera2()
//{
//	CWnd* pWnd = m_wndSplitter.GetPane(1, 0);
//	CConfigView* pView = DYNAMIC_DOWNCAST(CConfigView, pWnd);
//	return pView;
//}

void CManageDock::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	GetClientRect(rc);
	CBrush brush;
	brush.CreateSolidBrush(RGB(255, 255, 255));  
	dc.FillRect(&rc, &brush);
}

int CManageDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_wndManagePanel = CConfigView::CreateOne(this);

	//CRect cr;
	//GetClientRect(&cr);

 //    // create splitter window
 //    if (!m_wndSplitter.CreateStatic(this, 2, 1))
 //        return FALSE;

	// CSize size(300, cr.Height() / 2);
 //    m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CConfigView), size, NULL);
	// m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CConfigView), size, NULL);
		
	// m_wndSplitter2.CreateStatic(&m_wndSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 1));

	// if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CMCAView), CSize(100, cr.Height() / 2), pContext)  ||
    //     !m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CMCAView), CSize(100, cr.Height() / 2), pContext))
    // {
    //     m_wndSplitter2.DestroyWindow();
    //     return FALSE;
    // }

	return TRUE;
}
