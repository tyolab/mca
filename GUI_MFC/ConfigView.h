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
#include "afxcmn.h"
#include "afxwin.h"
#include "Resource.h"

// forwards
class CMCADoc;

// CConfigView form view
class CConfigView : public CFormView
{
    DECLARE_DYNCREATE(CConfigView)

public:
    CMCADoc* GetDocument();

    void FillDeviceListCtrl();
protected:
    CConfigView();           // protected constructor used by dynamic creation
    virtual ~CConfigView();

public:
	static CConfigView *CreateOne(CWnd *pParent);

public:
    enum { IDD = IDD_CONFIGURATION };
#ifdef _DEBUG
    virtual void AssertValid() const;
#ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif

private:
	CMCADoc* m_dummyDoc;
	int			 m_id;
public:
	CMCADoc* GetRealDoc() {
		return reinterpret_cast<CMCADoc*>(m_pDocument);
	}

	void SetID(int id) {
		m_id = id;
	}

	int GetID() {
		return m_id;
	}

    // Attributes
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

    void ComboClearItems(CComboBox*ctrl);
    void UpdateSlider(CSliderCtrl *ctrl, GenApi::IInteger* pInteger, int64_t min = -1, int64_t max = -1);
	void UpdateSlider(CSliderCtrl *ctrl, int64_t number, BOOL readable, BOOL writable, int64_t min = -1, int64_t max = -1);
    void UpdateSliderText( CStatic *ctrl, GenApi::IInteger* pInteger, int64_t defaultValue = -1 );
	void UpdateSliderText(CStatic *ctrl, uint64_t value, BOOL writable);
	void UpdateSliderText(CStatic *ctrl, BOOL writable, CString text);
    void UpdateEnumeration( CComboBox *ctrl, GenApi::IEnumeration* pEnum );

public:
	void UpdateDurationCtrls();
	void UpdateFrameRateCtrls();
	void UpdatePartnerViewCtrls();
	void UpdateGainCtrl();
	void UpdateEnumerationCtrl();
	void UpdateExposureTimeCtrl();
	void UpdateWidthCtrl();
	void UpdateHeightCtrl();
    
    DECLARE_MESSAGE_MAP()
private:
    // Device list control
    CListCtrl m_deviceListCtrl;

	CSliderCtrl m_ctrlWidthSlider;
	// Width text control
	CStatic m_ctrlWidthText;

	CSliderCtrl m_ctrlHeightSlider;
	// Height text control
	CStatic m_ctrlHeightText;

    // Exposure time slider
    CSliderCtrl m_ctrlExposureSlider;
    // Exposure time text control
    CStatic m_ctrlExposureText;
    // Gain slider
    CSliderCtrl m_ctrlGainSlider;
    // Gain text control
    CStatic m_ctrlGainText;

	CSliderCtrl m_ctrlFrameRateSlider;
	// FrameRate time text control
	CStatic m_ctrlFrameRateText;

	CSliderCtrl m_ctrlResultingFrSlider;
	// ResultingFr time text control
	CStatic m_ctrlResultingFrText;

	CSliderCtrl m_ctrlDurationSlider;
	// Duration time text control
	CStatic m_ctrlDurationText;

    // Test image selector
    CComboBox m_ctrlTestImage;
    // Pixel format selector
    CComboBox m_ctrlPixelFormat;
    // Remember the full name of the currently selected device so it can be restored in case of new enumeration.
    Pylon::String_t m_fullNameSelected;
    BOOL m_updatingList;

	CConfigView *m_ptrPartnerView;

public:
    int OnScroll( CScrollBar* pScrollBar, CSliderCtrl* pCtrl, GenApi::IInteger* pInteger, int64_t min = -1, int64_t max = -1, int64_t increment = -1);
	int OnScrollTo(CScrollBar* pScrollBar, CSliderCtrl* pCtrl, BOOL writable, int64_t value, int64_t minimum, int64_t maximum, int64_t increment);
	void SetPartnerView(CConfigView *partnerView);

    afx_msg void OnItemchangedDevicelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
    afx_msg void OnCbnSelendokTestimageCombo();
    afx_msg void OnCbnSelendokPixelformatCombo();
    afx_msg void OnCbnDropdownTestimageCombo();
    afx_msg void OnCbnDropdownPixelformatCombo();
};


#ifndef _DEBUG  // debug version in CConfigView.cpp
inline CMCADoc* CConfigView::GetDocument()
{
	if (NULL == m_pDocument)
		return m_dummyDoc;
    return reinterpret_cast<CMCADoc*>(m_pDocument);
}
#endif
