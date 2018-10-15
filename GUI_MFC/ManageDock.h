#pragma once
#include <afxdockablepane.h>
#include "ConfigView.h"

class CManageDock :
	public CDockablePane
{
	DECLARE_DYNAMIC(CManageDock)

private:
	CSplitterWnd m_wndSplitter;

public:
	CConfigView *m_wndManagePanel;
	
public:
	CManageDock();
	virtual ~CManageDock();

	CConfigView *getConfigView() {
		return m_wndManagePanel;
	}

	/*CConfigView *getConfigViewCamera();
	CConfigView *getConfigViewCamera2();*/

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

