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
    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();

// (Non-MFC function)
public:
    const Pylon::DeviceInfoList_t& GetDeviceInfoList() const;
    int EnumerateDevices();
    CString CMCAApp::SetDeviceFullName(LPCTSTR lpszFullDeviceName);
	void GetAllDocuments(DocsPtrList& listDocs);

private:

    // List of all attached devices
    Pylon::DeviceInfoList_t m_devices;
    CString m_strDeviceFullName;

	// make things simple
	// camera doc #1
	CMCADoc *m_cameraDoc1;

	// camera doc #2
	CMCADoc *m_cameraDoc2;

public:
    afx_msg void OnOpenCamera();
};

extern CMCAApp theApp;
