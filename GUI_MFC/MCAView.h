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

#include "MCADoc.h"

class CMCAView : public CScrollView
{
private:
	CMCADoc m_Doc;

protected: // create from serialization only
    CMCAView();
    DECLARE_DYNCREATE(CMCAView)

// Attributes
public:
    CMCADoc* GetDocument();

// Operations
public:

// Overrides
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
    virtual void OnInitialUpdate(); // called first time after construct
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

// Implementation
public:
    virtual ~CMCAView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG  // debug version in MCAView.cpp
inline CMCADoc* CMCAView::GetDocument() 
{
	return reinterpret_cast<CMCADoc*>(m_pDocument);
	//return &m_Doc;
}
#endif
