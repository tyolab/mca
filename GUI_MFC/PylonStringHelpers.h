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

#include <atlconv.h>
#include "windows.h"

#if !defined(UNICODE)
#error Only Unicode builds are supported.
#endif

// Auto conversion class: UTF-8 to wide character. See ATL/MFC String conversion in MSDN for further documentation.
class CUtf82W : public CA2W
{
public:
    CUtf82W(LPCSTR psz) throw(...) :
        CA2W(psz, CP_UTF8)
    {
    }
    
    
    CUtf82W(LPCSTR psz, UINT nCodePage) throw(...) :
        CA2W(psz, nCodePage)
    {
    }

private:
    CUtf82W(const CUtf82W&) throw();
    CUtf82W& operator=(const CUtf82W&) throw();
};

// Auto conversion class: wchar to UTF-8. See ATL/MFC String conversion in MSDN for further documentation.
class CW2Utf8 : public CW2A
{
public:
    CW2Utf8(LPCWSTR psz) throw(...) :
        CW2A(psz)
    {
    }

    
    CW2Utf8(LPCWSTR psz, UINT nCodePage) throw(...) :
        CW2A(psz, nCodePage)
    {
    }

private:
    CW2Utf8(const CW2Utf8&) throw();
    CW2Utf8& operator=(const CW2Utf8&) throw();
};


// Convert Pylon::String_t to CString.
CString GetCString(const Pylon::String_t& pylonString);

// Convert CString to pylon::String_t.
Pylon::String_t GetString_t(LPCTSTR pszWString);
