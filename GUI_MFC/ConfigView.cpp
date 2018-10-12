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

}


CConfigView::~CConfigView()
{
}


void CConfigView::DoDataExchange( CDataExchange* pDX )
{
    CFormView::DoDataExchange( pDX );
    DDX_Control( pDX, IDC_DEVICELIST, m_deviceListCtrl );
    DDX_Control( pDX, IDC_EXPOSURE_SLIDER, m_ctrlExposureSlider );
    DDX_Control( pDX, IDC_EXPOSURE_STATIC, m_ctrlExposureText );
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
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMCADoc)));
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
        // Display the current values.
        UpdateSlider(&m_ctrlExposureSlider, GetDocument()->GetExposureTime());
        UpdateSliderText(&m_ctrlExposureText, GetDocument()->GetExposureTime());

        UpdateSlider(&m_ctrlGainSlider, GetDocument()->GetGain());
        UpdateSliderText(&m_ctrlGainText, GetDocument()->GetGain());

        UpdateEnumeration(&m_ctrlTestImage, GetDocument()->GetTestImage());
        UpdateEnumeration(&m_ctrlPixelFormat, GetDocument()->GetPixelFormat());
    }
}

// Called to update value of slider.
void CConfigView::UpdateSlider( CSliderCtrl *pCtrl, GenApi::IInteger* pInteger )
{
    if (GenApi::IsReadable( pInteger ))
    {
        int64_t minimum = pInteger->GetMin();
        int64_t maximum = pInteger->GetMax();
        int64_t value = pInteger->GetValue();

        // Possible loss of data because Windows controls only supports 
        // 32-bitness while GenApi supports 64-bitness.
        pCtrl->SetRange( static_cast<int>(minimum), static_cast<int>(maximum) );
        pCtrl->SetPos( static_cast<int>(value) );
    }
    pCtrl->EnableWindow( GenApi::IsWritable( pInteger ) );
}

// Called to update the value of a label.
void CConfigView::UpdateSliderText( CStatic *pString, GenApi::IInteger* pInteger )
{
    if (GenApi::IsReadable( pInteger ))
    {
        // Set the value as a string in wide character format.
        pString->SetWindowText(CUtf82W(pInteger->ToString().c_str()));
    }
    else
    {
        pString->SetWindowText( _T( "n/a" ) );
    }
    pString->EnableWindow( GenApi::IsWritable( pInteger ) );
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
        int i = 0;
        for (Pylon::DeviceInfoList_t::const_iterator it = devices.begin(); it != devices.end(); ++it)
        {
            // Get the pointer to the current device info.
            const Pylon::CDeviceInfo* const pDeviceInfo = &(*it);

            // Add the item to the list.
            int nItem = m_deviceListCtrl.InsertItem( i++, CUtf82W( pDeviceInfo->GetFriendlyName() ) );

            // Remember the pointer to the device info.
            m_deviceListCtrl.SetItemData( nItem, (DWORD_PTR) pDeviceInfo );

            // Restore selection if necessary.
            if (pDeviceInfo->GetFullName() == fullNameSelected)
            {
                m_deviceListCtrl.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
                m_deviceListCtrl.SetSelectionMark( nItem );
            }
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
        if (theApp.SetDeviceFullName( strFullname ) != strFullname)
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
    nPos = OnScroll( pScrollBar, &m_ctrlExposureSlider, GetDocument()->GetExposureTime() );
    nPos = OnScroll( pScrollBar, &m_ctrlGainSlider, GetDocument()->GetGain() );

    CFormView::OnHScroll( nSBCode, nPos, pScrollBar );
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
UINT CConfigView::OnScroll( CScrollBar* pScrollBar, CSliderCtrl* pCtrl, GenApi::IInteger* pInteger )
{
    if (pScrollBar->GetSafeHwnd() == pCtrl->GetSafeHwnd())
    {
        if (GenApi::IsWritable( pInteger ))
        {   
            // Fetch current value, range, and increment of the camera feature.
            int64_t value = pInteger->GetValue();
            const int64_t minimum = pInteger->GetMin();
            const int64_t maximum = pInteger->GetMax();
            const int64_t increment = pInteger->GetInc();

            // Adjust the pointer to the slider to get the correct position.
            int64_t newvalue = 0;
            CSliderCtrl* pSlider = DYNAMIC_DOWNCAST( CSliderCtrl, pScrollBar );
            newvalue = pSlider->GetPos();

            // Round to the next valid value.
            int64_t roundvalue = RoundTo( newvalue, value, minimum, maximum, increment );
            if (roundvalue == value)
            {
                return 0;
            }

            // Try to set the value. If successful, update the scroll position.
            try
            {
                pInteger->SetValue( roundvalue );
                pSlider->SetPos( (int) roundvalue );
            }
            catch (GenICam::GenericException &e)
            {
                UNUSED( e );
                TRACE( "Failed to set '%s':%s", pInteger->GetNode()->GetDisplayName().c_str(), e.GetDescription() );
            }
            catch (...)
            {
                TRACE( "Failed to set '%s'", pInteger->GetNode()->GetDisplayName().c_str() );
            }

            return static_cast<UINT>(value);
        }
    }

    return 0;
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
