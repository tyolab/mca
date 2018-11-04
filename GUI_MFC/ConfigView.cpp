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
#include "MCA.h"

#include "MCADoc.h"
#include "ConfigView.h"

static const int RESOLUTION_MIN = 320;
static const int RESOLUTION_MAX = 3200;
static const int RESOLUTION_INC = 32;

static const int FRAME_RATE_MIN = 25;
static const int FRAME_RATE_MAX = 600;

static const int RESULTING_FR_MIN = 25;
static const int RESULTING_FR_MAX = 600;

static const int EXPOSURE_TIME_MIN = 1;
static const int EXPOSURE_TIME_MAX = 3000;

static const int GAIN_MIN = 0;
static const int GAIN_MAX = 1000;

static const int DURATION_MIN = 1;
static const int DURATION_MAX = 5;

// Stores GenApi enumeration items into MFC ComboBox
void FillEnumerationListCtrl( GenApi::IEnumeration* pEnum, CComboBox* pCtrl )
{
    typedef GenApi::StringList_t::iterator Iterator_t;
    const GenApi::IEnumEntry *pCurrent = pEnum->GetCurrentEntry();
    int currentIndex = -1;
    GenApi::StringList_t slist;

    if (GenApi::IsReadable( pEnum))
    {
        // Retrieve the list of entries with nicely formatted names.
        pEnum->GetSymbolics( slist );


        // Add items if not already present.
        for (Iterator_t it = slist.begin(), end = slist.end(); it != end; ++it)
        {
            CString item = CUtf82W( it->c_str() );
            if (pCtrl->FindStringExact( 0, item ) >= 0)
            {
                continue;
            }
            // Set the name in wide character format.
            const int index = pCtrl->AddString( CUtf82W( it->c_str() ) );
            const GenApi::IEnumEntry *pEntry = pEnum->GetEntryByName( *it );
            // Store the pointer for easy node access.
            pCtrl->SetItemData( index, reinterpret_cast<DWORD_PTR>(pEntry) );
            if (pEntry == pCurrent)
            {
                currentIndex = index;
            }
        }
    }
}

// CConfigView
IMPLEMENT_DYNCREATE( CConfigView, CFormView )


CConfigView::CConfigView()
: CFormView( CConfigView::IDD )
, m_updatingList( FALSE )
{
	m_dummyDoc = new CMCADoc;
	m_id = -1;
}


CConfigView::~CConfigView()
{
	delete m_dummyDoc;
}

CConfigView * CConfigView::CreateOne(CWnd * pParent)
{
	CConfigView *p_ManagePanel = new CConfigView;

	CRect rc;
	pParent->GetWindowRect(&rc);
	//CMyFormView *pFormView = NULL; 
   //CRuntimeClass *pRuntimeClass = RUNTIME_CLASS(CMyFormView); 
   //pFormView = (CMyFormView *)pRuntimeClass->CreateObject(); 

   //CDockableFormViewAppDoc *pDoc = CDockableFormViewAppDoc::CreateOne();////////////////////////////////////////68 //pFormView->m_pDocument = pDoc;//////////////////////////////////////////////////////////////////////////////69 CCreateContext *pContext = NULL;
#if 0 
	if (!p_ManagePanel->CreateEx(0, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 205, 157),
		pParent, -1, nullptr))
#else 
	if (!p_ManagePanel->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, rc.Width(), rc.Height()), pParent, AFX_IDW_PANE_FIRST, NULL))
#endif 
		//if( !pFormView->CreateEx( 0, AfxRegisterWndClass(0, 0, 0, 0), NULL, 
		// WS_CHILD | WS_VISIBLE, CRect( 0, 0, 205, 157), pParent, -1, pContext) ) 
	{
		AfxMessageBox(_T("Failed in creating CMyFormView"));

	}
	// p_ManagePanel->OnInitialUpdate();
	return p_ManagePanel;
}


void CConfigView::DoDataExchange( CDataExchange* pDX )
{
    CFormView::DoDataExchange( pDX );
    DDX_Control( pDX, IDC_DEVICELIST, m_deviceListCtrl );
	DDX_Control(pDX, IDC_WIDTH_SLIDER, m_ctrlWidthSlider);
	DDX_Control(pDX, IDC_WIDTH_STATIC, m_ctrlWidthText);
    DDX_Control( pDX, IDC_HEIGHT_SLIDER, m_ctrlHeightSlider );
    DDX_Control( pDX, IDC_HEIGHT_STATIC, m_ctrlHeightText );
	DDX_Control(pDX, IDC_FRAME_RATE_SLIDER, m_ctrlFrameRateSlider);
	DDX_Control(pDX, IDC_FRAME_RATE_STATIC, m_ctrlFrameRateText);
	DDX_Control(pDX, IDC_RESULTING_FR_SLIDER, m_ctrlResultingFrSlider);
	DDX_Control(pDX, IDC_RESULTING_FR_STATIC, m_ctrlResultingFrText);
	DDX_Control(pDX, IDC_EXPOSURE_SLIDER, m_ctrlExposureSlider);
	DDX_Control(pDX, IDC_EXPOSURE_STATIC, m_ctrlExposureText);
	DDX_Control(pDX, IDC_DURATION_SLIDER, m_ctrlDurationSlider);
	DDX_Control(pDX, IDC_DURATION_STATIC, m_ctrlDurationText);
    DDX_Control( pDX, IDC_GAIN_SLIDER, m_ctrlGainSlider );
    DDX_Control( pDX, IDC_GAIN_STATIC, m_ctrlGainText );
    DDX_Control( pDX, IDC_TESTIMAGE_COMBO, m_ctrlTestImage );
    DDX_Control( pDX, IDC_PIXELFORMAT_COMBO, m_ctrlPixelFormat );
}


BEGIN_MESSAGE_MAP( CConfigView, CFormView )
    ON_NOTIFY( LVN_ITEMCHANGED, IDC_DEVICELIST, &CConfigView::OnItemchangedDevicelist )
    ON_WM_HSCROLL()
    ON_CBN_SELENDOK( IDC_TESTIMAGE_COMBO, &CConfigView::OnCbnSelendokTestimageCombo )
    ON_CBN_SELENDOK( IDC_PIXELFORMAT_COMBO, &CConfigView::OnCbnSelendokPixelformatCombo )
    ON_CBN_DROPDOWN( IDC_TESTIMAGE_COMBO, &CConfigView::OnCbnDropdownTestimageCombo )
    ON_CBN_DROPDOWN( IDC_PIXELFORMAT_COMBO, &CConfigView::OnCbnDropdownPixelformatCombo )
END_MESSAGE_MAP()

// CConfigView diagnostics
#ifdef _DEBUG
void CConfigView::AssertValid() const
{
    CFormView::AssertValid();
}


CMCADoc* CConfigView::GetDocument() // non-debug version is inline
{
	// the config view is not associated with te camera view yet
	 if (NULL == m_pDocument || !(m_pDocument->IsKindOf(RUNTIME_CLASS(CMCADoc))))
		return m_dummyDoc;
    return (CMCADoc*)m_pDocument;
}

#ifndef _WIN32_WCE
void CConfigView::Dump( CDumpContext& dc ) const
{
    CFormView::Dump( dc );
}
#endif
#endif //_DEBUG


void CConfigView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    EUpdateHint eHint = (EUpdateHint)lHint;

    if (eHint == UpdateHint_All || (eHint & UpdateHint_DeviceList))
    {
        FillDeviceListCtrl();
    }

    if (eHint == UpdateHint_All || (eHint & UpdateHint_Feature))
    {
		UpdateWidthCtrl();

		UpdateHeightCtrl();

        // Display the current values.
		UpdateExposureTimeCtrl();

		UpdateGainCtrl();

        UpdateEnumeration(&m_ctrlTestImage, GetDocument()->GetTestImage());
		UpdateEnumerationCtrl();
        
		UpdateFrameRateCtrls();
		UpdateDurationCtrls();
    }
}

void CConfigView::UpdateDurationCtrls()
{
	BOOL readable, writable;
	readable = writable = GetDocument() != m_dummyDoc;
	UpdateSlider(&m_ctrlDurationSlider, CMCADoc::GetDuration(), readable, writable, DURATION_MIN, DURATION_MAX);
	UpdateSliderText(&m_ctrlDurationText, CMCADoc::GetDuration(), writable);
}

void CConfigView::UpdateFrameRateCtrls()
{
	BOOL readable, writable;
	readable = writable = GetDocument() != m_dummyDoc;

	UpdateSlider(&m_ctrlFrameRateSlider, GetDocument()->GetFrameRateValue(), readable, writable, FRAME_RATE_MIN, FRAME_RATE_MAX);
	UpdateSliderText(&m_ctrlFrameRateText, GetDocument()->GetFrameRate(), GetDocument()->GetFrameRateValue());

	UpdateSlider(&m_ctrlResultingFrSlider, -1, FALSE, FALSE, RESULTING_FR_MIN, RESULTING_FR_MAX);
	UpdateSliderText(&m_ctrlResultingFrText, GetDocument()->GetResultingFr(), GetDocument()->GetResultingFrValue());
}

void CConfigView::UpdatePartnerViewCtrls()
{
	if (NULL != m_ptrPartnerView) {
		m_ptrPartnerView->UpdateDurationCtrls();
		m_ptrPartnerView->Invalidate();
	}
}

void CConfigView::UpdateGainCtrl()
{
	UpdateSlider(&m_ctrlGainSlider, GetDocument()->GetGain(), GAIN_MIN, GAIN_MAX);
	UpdateSliderText(&m_ctrlGainText, GetDocument()->GetGain());
}

void CConfigView::UpdateEnumerationCtrl()
{
	UpdateEnumeration(&m_ctrlPixelFormat, GetDocument()->GetPixelFormat());
}

void CConfigView::UpdateExposureTimeCtrl()
{
	UpdateSlider(&m_ctrlExposureSlider, GetDocument()->GetExposureTime(), EXPOSURE_TIME_MIN, EXPOSURE_TIME_MAX);
	UpdateSliderText(&m_ctrlExposureText, GetDocument()->GetExposureTime());
}

void CConfigView::UpdateWidthCtrl()
{
	UpdateSlider(&m_ctrlWidthSlider, GetDocument()->GetWidth(), RESOLUTION_MIN, RESOLUTION_MAX);
	UpdateSliderText(&m_ctrlWidthText, GetDocument()->GetWidth());
}

void CConfigView::UpdateHeightCtrl()
{
	UpdateSlider(&m_ctrlHeightSlider, GetDocument()->GetHeight(), RESOLUTION_MIN, RESOLUTION_MAX);
	UpdateSliderText(&m_ctrlHeightText, GetDocument()->GetHeight());
}

// Called to update value of slider.
void CConfigView::UpdateSlider( CSliderCtrl *pCtrl, GenApi::IInteger* pInteger, int64_t min, int64_t max)
{
    if (GenApi::IsReadable( pInteger ))
    {
        int64_t minimum = min != -1 ? min: pInteger->GetMin();
        int64_t maximum = max != -1 ? max : pInteger->GetMax();
        int64_t value = pInteger->GetValue();

        // Possible loss of data because Windows controls only supports 
        // 32-bitness while GenApi supports 64-bitness.
        pCtrl->SetRange( static_cast<int>(minimum), static_cast<int>(maximum) );
        pCtrl->SetPos( static_cast<int>(value) );
    }
    pCtrl->EnableWindow( GenApi::IsWritable( pInteger ) );
}


// Called to update value of slider.
void CConfigView::UpdateSlider(CSliderCtrl *pCtrl, int64_t number, BOOL readable, BOOL writable, int64_t minimum, int64_t maximum)
{
	if (readable)
	{
		// Possible loss of data because Windows controls only supports 
		// 32-bitness while GenApi supports 64-bitness.
		pCtrl->SetRange(static_cast<int>(minimum), static_cast<int>(maximum));
		pCtrl->SetPos(static_cast<int>(number));
	}
	pCtrl->EnableWindow(writable);
}

// Called to update the value of a label.
void CConfigView::UpdateSliderText( CStatic *pString, GenApi::IInteger* pInteger, int64_t defaultValue)
{
	CString text;
	BOOL readable = GenApi::IsReadable(pInteger);
	if (readable)
		text = CUtf82W(pInteger->ToString().c_str());
	else if (defaultValue != -1) {
		text.Format(_T("%I64d"), defaultValue);
		readable = TRUE;
	}
	else
		text = "n/a";
    pString->EnableWindow( GenApi::IsWritable( pInteger ) );
	UpdateSliderText(pString, readable, text);
}

// Called to update the value of a label.
void CConfigView::UpdateSliderText(CStatic *pString, uint64_t value, BOOL writable)
{
	CString strValue;
	strValue.Format(_T("%d"), value);
	pString->SetWindowText(strValue);

	pString->EnableWindow(writable);
}

void CConfigView::UpdateSliderText(CStatic * pString, BOOL readable, CString text)
{
	if (readable)
	{
		// Set the value as a string in wide character format.
		pString->SetWindowText(text);
	}
	else
	{
		pString->SetWindowText(_T("n/a"));
	}
}


// Called to update the enumeration in a combo box.
void CConfigView::UpdateEnumeration( CComboBox *pCtrl, GenApi::IEnumeration* pEnum )
{
    pCtrl->EnableWindow(GenApi::IsWritable(pEnum));

    if (GenApi::IsReadable(pEnum))
    {
        GenApi::IEnumEntry* pEnumEntry = pEnum->GetCurrentEntry();


        if (pCtrl->GetCount())
        {   // Compare the current entry with the current selection.
            int selected = pCtrl->GetCurSel();
            if (pEnumEntry == pCtrl->GetItemDataPtr( selected ))
            {
                return;  // Shortcut: the current value is already displayed
            }

            pCtrl->ResetContent();

        }

        // Add the current item.
        // Set the name in wide character format.
        int curIndex = pCtrl->AddString( CUtf82W( pEnumEntry->GetSymbolic().c_str() ) );
        // Store the pointer for easy node access.
        pCtrl->SetItemData( curIndex, reinterpret_cast<DWORD_PTR>(pEnumEntry) );
        pCtrl->SetCurSel( curIndex );
    }

}

void CConfigView::FillDeviceListCtrl()
{
    // Remember selection before deleting items so it can be restored after refilling.
    const Pylon::String_t fullNameSelected = m_fullNameSelected;

    ASSERT( !m_updatingList );

    m_updatingList = TRUE;

    // Delete the device list items before refilling.
    m_deviceListCtrl.DeleteAllItems();

    const Pylon::DeviceInfoList_t& devices = theApp.GetDeviceInfoList();
    if (!devices.empty())
    {
        int i = 0, count = 0;
		for (Pylon::DeviceInfoList_t::const_iterator it = devices.begin(); it != devices.end(); ++it)
		{
			// Get the pointer to the current device info.
			const Pylon::CDeviceInfo* const pDeviceInfo = &(*it);

			if (m_id > -1 && count == m_id) {
				// Add the item to the list.
				int nItem = m_deviceListCtrl.InsertItem(i++, CUtf82W(pDeviceInfo->GetFriendlyName()));

				// Remember the pointer to the device info.
				m_deviceListCtrl.SetItemData(nItem, (DWORD_PTR)pDeviceInfo);

				// Restore selection if necessary.
				if (pDeviceInfo->GetFullName() == fullNameSelected)
				{
					m_deviceListCtrl.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
					m_deviceListCtrl.SetSelectionMark(nItem);
				}
			}
			++count;
        }
    }

    m_updatingList = FALSE;
}

// CConfigView message handlers

// Device list was changed. 
void CConfigView::OnItemchangedDevicelist( NMHDR *pNMHDR, LRESULT *pResult )
{
    if (m_updatingList)
    {
        *pResult = 0;
        return;
    }

    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    // Check if device selection was changed.
    int iItem = pNMLV->iItem;

    if (!(LVIF_STATE & pNMLV->uChanged))
    {
        *pResult = 0;
        return;
    }

    if (LVIS_SELECTED & pNMLV->uNewState)
    {
        // A device was selected.
        // Remember the full name of the currently selected device so it can be restored in case of new enumeration.
        const Pylon::CDeviceInfo* const pSelectedDeviceInfo = (const Pylon::CDeviceInfo*)m_deviceListCtrl.GetItemData( iItem );
        ASSERT( pSelectedDeviceInfo != NULL );

        m_fullNameSelected = pSelectedDeviceInfo ? pSelectedDeviceInfo->GetFullName() : "";

        // To use the MFC document/view services, we need to pass the full name to the app class.
        // This way, the app class can pass it to the OpenDocumentFile function.
        CString strFullname(CUtf82W( m_fullNameSelected.c_str() ));
        if (theApp.SetDeviceFullName( strFullname, GetID() ) != strFullname)
        {
            theApp.GetMainWnd()->PostMessage( WM_COMMAND, MAKEWPARAM( ID_OPEN_CAMERA, 0 ), 0 );
        }
    }
    else if (LVIS_SELECTED & pNMLV->uOldState)
    {
        m_deviceListCtrl.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
    }

    *pResult = 0;
}

// Called to update the slider controls.
void CConfigView::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    // Forward the scroll message to the slider controls.
	UINT oldValue = 0;
	int newValue = nPos;

	oldValue = GetDocument()->GetFrameRateValue();
	newValue = OnScrollTo(pScrollBar, &m_ctrlFrameRateSlider, TRUE, GetDocument()->GetFrameRateValue(), FRAME_RATE_MIN, FRAME_RATE_MAX, 1);
	if (newValue > 0 && oldValue != newValue) {
		GetDocument()->SetFrameRateValue(newValue);
		if (NULL != m_ptrPartnerView)
			m_ptrPartnerView->GetDocument()->SetFrameRateValue(newValue);
	}

	oldValue = GetDocument()->GetExposureTimeValue();
    newValue =  OnScroll( pScrollBar, &m_ctrlExposureSlider, GetDocument()->GetExposureTime() , EXPOSURE_TIME_MIN, EXPOSURE_TIME_MAX);
	if (newValue > 0 && oldValue != newValue) {
		if (NULL != m_ptrPartnerView)
			m_ptrPartnerView->GetDocument()->GetExposureTime()->SetValue(newValue);
	}

	oldValue = GetDocument()->GetGainValue();
    newValue =  OnScroll( pScrollBar, &m_ctrlGainSlider, GetDocument()->GetGain() , GAIN_MIN, GAIN_MAX);
	if (newValue > 0 && oldValue != newValue) {
		if (NULL != m_ptrPartnerView)
			m_ptrPartnerView->GetDocument()->GetGain()->SetValue(newValue);
	}

	/*oldValue = GetDocument()->Get
	newValue =  OnScroll(pScrollBar, &m_ctrlResultingFrSlider, GetDocument()->GetResultingFr(), FRAME_RATE_MIN, FRAME_RATE_MAX);
	if (newValue > 0 && oldValue != newValue) {

	}*/

	oldValue = GetDocument()->GetHeightValue();
	newValue =  OnScroll(pScrollBar, &m_ctrlHeightSlider, GetDocument()->GetHeight(), RESOLUTION_MIN, RESOLUTION_MAX);
	if (newValue > 0 && oldValue != newValue) {
		if (NULL != m_ptrPartnerView)
			m_ptrPartnerView->GetDocument()->GetHeight()->SetValue(newValue);
	}

	oldValue = GetDocument()->GetWidthValue();
	newValue =  OnScroll(pScrollBar, &m_ctrlWidthSlider, GetDocument()->GetWidth(), RESOLUTION_MIN, RESOLUTION_MAX);
	if (newValue > 0 && oldValue != newValue) {
		if (NULL != m_ptrPartnerView)
			m_ptrPartnerView->GetDocument()->GetWidth()->SetValue(newValue);
	}

	oldValue = GetDocument()->GetDuration();
	newValue =  OnScrollTo(pScrollBar, &m_ctrlDurationSlider, TRUE, GetDocument()->GetDuration(), DURATION_MIN, DURATION_MAX, 1);

	if (newValue > 0 && oldValue != newValue) {
		CMCADoc::SetDuration(newValue);
		UpdateDurationCtrls();
	}
	// UpdateFrameRateCtrls();

    CFormView::OnHScroll( nSBCode, newValue, pScrollBar );

	// refresh even the window is not focused
	GetDocument()->UpdateSettingsDisplay();
	// Update Partner View too
	if (NULL != m_ptrPartnerView)
		m_ptrPartnerView->GetDocument()->UpdateSettingsDisplay();
}

// Round a value to a valid value
int64_t RoundTo( int64_t newValue, int64_t oldValue, int64_t minimum, int64_t maximum, int64_t increment )
{
    const int Direction = (newValue - oldValue) > 0 ? 1 : -1;
    const int64_t nIncr = (newValue - minimum) / increment;

    switch (Direction)
    {
    case 1: // Up
        return min( maximum, minimum + nIncr*increment );
    case -1: // Down
        return max( minimum, minimum + nIncr*increment );
    }
    return newValue;
}

// Update a slider and set a valid value.
int CConfigView::OnScroll(CScrollBar* pScrollBar, CSliderCtrl* pCtrl, GenApi::IInteger* pInteger, int64_t min, int64_t max, int64_t inc)
{
	BOOL writable = GenApi::IsWritable(pInteger);
	int64_t value = 0;

	if (writable) {
		value = pInteger->GetValue();
		const int64_t minimum = min != -1 ? min : pInteger->GetMin();
		const int64_t maximum = max != -1 ? max : pInteger->GetMax();
		const int64_t increment = inc != -1 ? inc : pInteger->GetInc();

		// Try to set the value. If successful, update the scroll position.
		try
		{
			int64_t newValue = OnScrollTo(pScrollBar, pCtrl, writable, value, minimum, maximum, increment);
			if (newValue != -1 && newValue != value) {
				pInteger->SetValue(newValue);
				return newValue;
			}
		}
		catch (GenICam::GenericException &e)
		{
			UNUSED(e);
			TRACE("Failed to set '%s':%s", pInteger->GetNode()->GetDisplayName().c_str(), e.GetDescription());
		}
		catch (...)
		{
			TRACE("Failed to set '%s'", pInteger->GetNode()->GetDisplayName().c_str());
		}
	}
	return value;
}
int CConfigView::OnScrollTo(CScrollBar* pScrollBar, CSliderCtrl* pCtrl, BOOL writable, int64_t value, int64_t minimum, int64_t maximum, int64_t increment)
{
    if (pScrollBar->GetSafeHwnd() == pCtrl->GetSafeHwnd())
    {
        if (writable)
        {   
            // Fetch current value, range, and increment of the camera feature.


            // Adjust the pointer to the slider to get the correct position.
            int64_t newvalue = 0;
            CSliderCtrl* pSlider = DYNAMIC_DOWNCAST( CSliderCtrl, pScrollBar );
            newvalue = pSlider->GetPos();

            // Round to the next valid value.
            int64_t roundvalue = RoundTo( newvalue, value, minimum, maximum, increment );
            if (roundvalue == value)
            {
                return roundvalue;
            }

			pSlider->SetPos((int)roundvalue);
            return static_cast<UINT>(roundvalue);
        }
    }
	//if (pCtrl == &m_ctrlDurationSlider)
	//	return 1;
    return value;
}

void CConfigView::SetPartnerView(CConfigView * partnerView)
{
	m_ptrPartnerView = partnerView;
}

// Called when a test image is selected. Sets the new value.
void CConfigView::OnCbnSelendokTestimageCombo()
{
    // Update the camera with the value selected.
    const int selected = m_ctrlTestImage.GetCurSel();
    GenApi::IEnumEntry* pEntry = reinterpret_cast<GenApi::IEnumEntry*>(m_ctrlTestImage.GetItemDataPtr( selected ));
    GenApi::IEnumeration* pEnumeration = GetDocument()->GetTestImage();
    if (GenApi::IsWritable( pEnumeration ))
    {   // Try to update the test image.
        try 
        {
            pEnumeration->SetIntValue( pEntry->GetValue() );
        }
        catch (GenICam::GenericException &e)
        {
            UNUSED( e );
            TRACE( "Failed to set '%s':%s", pEnumeration->GetNode()->GetDisplayName().c_str(), e.GetDescription() );
        }
        catch (...)
        {
            TRACE( "Failed to set '%s'", pEnumeration->GetNode()->GetDisplayName().c_str() );
        }
    }
}

// Called when a pixel format is selected. Sets the new value.
void CConfigView::OnCbnSelendokPixelformatCombo()
{
    // Update the camera with the value selected.
    const int selected = m_ctrlPixelFormat.GetCurSel();
    GenApi::IEnumEntry* pEntry = reinterpret_cast<GenApi::IEnumEntry*>(m_ctrlPixelFormat.GetItemDataPtr( selected ));
    GenApi::IEnumeration* pEnumeration = GetDocument()->GetPixelFormat();
    if (GenApi::IsWritable( pEnumeration ))
    {   // Try to update the pixel format.
        try
        {
            pEnumeration->SetIntValue( pEntry->GetValue() );
			if (nullptr != m_ptrPartnerView) {
				m_ptrPartnerView->GetDocument()->GetPixelFormat()->SetIntValue( pEntry->GetValue() );
				m_ptrPartnerView->GetDocument()->UpdateSettingsDisplay();
			}
        }
        catch (GenICam::GenericException &e)
        {
            UNUSED( e );
            TRACE( "Failed to set '%s':%s", pEnumeration->GetNode()->GetDisplayName().c_str(), e.GetDescription() );
        }
        catch (...)
        {
            TRACE( "Failed to set '%s'", pEnumeration->GetNode()->GetDisplayName().c_str() );
        }
    }
}

// Called when the combo box drops down. Fills the drop down list with all available test images.
void CConfigView::OnCbnDropdownTestimageCombo()
{
    // Update the entries in the dropdown list.
    FillEnumerationListCtrl( GetDocument()->GetTestImage(), &m_ctrlTestImage );
}

// Called when the combo box drops down. Fills the drop down list with all available pixel formats.
void CConfigView::OnCbnDropdownPixelformatCombo()
{
    // Update the entries in the dropdown list.
    FillEnumerationListCtrl( GetDocument()->GetPixelFormat(), &m_ctrlPixelFormat );
}
