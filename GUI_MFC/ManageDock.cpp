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
	return TRUE;
}
