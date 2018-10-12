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

// CMCAApp:
// See GuiSample.cpp for the implementation of this class
//
class CMCAApp : public CWinApp
{
public:
    CMCAApp();

// Overrides
public:
    virtual BOOL InitInstance();

// Implementation
    afx_msg void OnAppAbout();
    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();

// (Non-MFC function)
public:
    const Pylon::DeviceInfoList_t& GetDeviceInfoList() const;
    int EnumerateDevices();
    CString CMCAApp::SetDeviceFullName(LPCTSTR lpszFullDeviceName);

private:

    // List of all attached devices
    Pylon::DeviceInfoList_t m_devices;
    CString m_strDeviceFullName;

public:
    afx_msg void OnOpenCamera();
};

extern CMCAApp theApp;
