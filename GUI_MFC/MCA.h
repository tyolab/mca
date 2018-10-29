/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    
    Defines the class behaviors for the application.
*/
#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

typedef CTypedPtrList<CObList, CDocument*> DocsPtrList;

class CMCADoc;

#include "CameraInfo.h"

// CMCAApp:
// See MCA.cpp for the implementation of this class
//
class CMCAApp : public CWinAppEx
{
public:
	static const size_t c_maxCamerasToUse = 2;

public:
    CMCAApp();

// Overrides
public:
    virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

    afx_msg void OnAppAbout();
	afx_msg void OnGrabOne();
	afx_msg void OnStartGrabbing();
	afx_msg void OnStopGrab();
	afx_msg void OnNewGrabresultCamera1();
	afx_msg void OnNewGrabresultCamera2();
	afx_msg void OnUpdateGrabOne(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStartGrabbing(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStopGrab(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileImageSaveAs(CCmdUI *pCmdUI);

    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();

// (Non-MFC function)
public:
    const Pylon::DeviceInfoList_t& GetDeviceInfoList() const;
    int EnumerateDevices();
    CString CMCAApp::SetDeviceFullName(LPCTSTR lpszFullDeviceName, int id = -1);
	void GetAllDocuments(DocsPtrList& listDocs);
	

private:

    // List of all attached devices
    Pylon::DeviceInfoList_t m_devices;
    CString m_strDeviceFullName;
	int		m_currentDeviceID; // 0, 1, ...

	// make things simple
	// camera info #1
	CCameraInfo m_cameraInfo1;

	// camera info #2
	CCameraInfo m_cameraInfo2;

	// video output folder
	CString m_strOutputFolder;

public:
    afx_msg void OnOpenCamera();
};

extern CMCAApp theApp;
