/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    
    This sample illustrates the use of a MFC GUI together with the pylon C++ API to enumerate the attached cameras, to
    configure a camera, to start and stop the grab and to display and store grabbed images. 
    It shows how to use GUI controls to display and modify camera parameters.
*/
#include "stdafx.h"
#include "PylonStringHelpers.h"


CString GetCString(const Pylon::String_t& pylonString)
{
    // Pylon strings are encoded in UTF-8.
    // Windows can't use UTF-8, so we must convert them TCHAR.

    // First convert UTF-8 char to wchar_t.
    CUtf82W pszWString(pylonString.c_str());

    // If necessary, CString constructor will convert from WCHAR to ANSI.
    return CString(pszWString);
}

// Convert a MFC CString/LPCTSTR to a Pylon::String_t
Pylon::String_t GetString_t(LPCTSTR pszString)
{
    if (pszString == NULL)
    {
        return Pylon::String_t();
    }

    // Windows can only convert from wchar to UTF-8,
    // so we must first convert from TCHAR to wchar.
    // If we are building for unicode, this is a noop.
    CT2CW pszWString(pszString);

    // Convert wchar to UTF-8.
    CW2Utf8 pszUtf8String(pszWString);

    return Pylon::String_t(pszUtf8String);
}
