/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    
    This sample illustrates the use of a MFC GUI together with the pylon C++ API to enumerate the attached cameras, to
    configure a camera, to start and stop the grab and to display and store grabbed images. 
    It shows how to use GUI controls to display and modify camera parameters.
*/
#pragma once

#include "ManageDock.h"

class CMCAView;

class CMainFrame : public CMDIFrameWndEx
{
    
// create from serialization only
    DECLARE_DYNCREATE(CMainFrame)

public:
	CMainFrame();

// Attributes
protected:
    CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitter2;
public:

// Operations
public:

// Overrides
public:
    //virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// Implementation
public:
    virtual ~CMainFrame();
    CMCAView* GetRightPane();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

	CConfigView* getConfigView() {
		return m_wndManageDock.getConfigView();
	}

protected:  // control bar embedded members
    CMFCStatusBar  m_wndStatusBar;
    CMFCToolBar    m_wndToolBar;
	//CConfigView m_wndConfigView;
	CManageDock m_wndManageDock;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
};
