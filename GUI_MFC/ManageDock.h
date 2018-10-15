#pragma once
#include <afxdockablepane.h>
#include "ConfigView.h"

class CManageDock :
	public CDockablePane
{
	DECLARE_DYNAMIC(CManageDock)

public:
	CConfigView *m_wndManagePanel;
	
public:
	CManageDock();
	virtual ~CManageDock();

	CConfigView *getConfigView() {
		return m_wndManagePanel;
	}

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

