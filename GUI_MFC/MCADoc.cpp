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

#include "ImageResult.h"
#include "MCADoc.h"
#include "AutoPacketSizeConfiguration.h"

#include <ctime>

using namespace Pylon;
using namespace GenApi;

const UINT DEFAULT_BUFFER_SIZE = 1000; // for example, 200fps, the buffer can hold 50s of recording

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMCADoc
IMPLEMENT_DYNCREATE(CMCADoc, CDocument)

BEGIN_MESSAGE_MAP(CMCADoc, CDocument)
    //ON_COMMAND(ID_CAMERA_GRABONE, &CMCADoc::OnGrabOne)
    //ON_UPDATE_COMMAND_UI(ID_CAMERA_GRABONE, &CMCADoc::OnUpdateGrabOne)
    //ON_COMMAND(ID_CAMERA_STARTGRABBING, &CMCADoc::OnStartGrabbing)
    //ON_UPDATE_COMMAND_UI(ID_CAMERA_STARTGRABBING, &CMCADoc::OnUpdateStartGrabbing)
    //ON_COMMAND(ID_CAMERA_STOPGRAB, &CMCADoc::OnStopGrab)
    //ON_UPDATE_COMMAND_UI(ID_CAMERA_STOPGRAB, &CMCADoc::OnUpdateStopGrab)
	//ON_UPDATE_COMMAND_UI(ID_FILE_IMAGE_SAVE_AS, &CMCADoc::OnUpdateFileImageSaveAs)

    ON_COMMAND(ID_NEW_GRABRESULT, &CMCADoc::OnNewGrabresult)
    ON_COMMAND(ID_VIEW_REFRESH, &CMCADoc::OnViewRefresh)
    ON_COMMAND(ID_FILE_SAVE_AS, &CMCADoc::OnFileSaveAs)
    ON_COMMAND(ID_FILE_IMAGE_SAVE_AS, &CMCADoc::OnFileImageSaveAs)

    ON_COMMAND( ID_UPDATE_NODES, &CMCADoc::OnUpdateNodes )
END_MESSAGE_MAP()

UINT CMCADoc::m_duration = 1;
UINT CMCADoc::m_bufferSize = DEFAULT_BUFFER_SIZE;
UINT CMCADoc::m_fps = 0;

// CMCADoc construction/destruction
CMCADoc::CMCADoc()
	: m_cntGrabbedImages(0)
	, m_cntDroppedImages(0)
	, m_cntSkippedImages(0)
	, m_cntGrabErrors(0)
	, m_hTestImage(NULL)
	, m_hGain(NULL)
	, m_hExposureTime(NULL)
	, m_hWidth(NULL)
	, m_hHeight(NULL)
	, m_hFrameRate(NULL)
	, m_cameraReady(FALSE)
	, m_camera(new CInstantCamera())
{
    // TODO: add one-time construction code here
	m_id = -1;
}


CMCADoc::~CMCADoc()
{
}

// Initial one-time initialization
BOOL CMCADoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

	return RegisterListeners();
}


// Some features of USB cameras are of type float (i.e. Gain).
// To use the value in Windows controls we need to convert it to an integer.
// This conversion can be done by the GenICam.
// All float features have a so called alias feature representing the float value
// converted to an integer. This function retrieves the Alias feature for the feature passed
// If the feature passed is already of type integer it will just return the
// value passed converted to IInteger.
// If the feature passed doesn't have an alias and is not an integer the function will return NULL.
GenApi::IInteger* CMCADoc::GetIntegerFeature(GenApi::INode* pNode) const
{
    if (pNode == NULL)
    {
        return NULL;
    }
    
    GenApi::CFloatPtr ptrFloat = pNode;
    GenApi::CIntegerPtr ptrInteger;

    // Is this a float feature?
    if (ptrFloat.IsValid())
    {
        // Get the alias feature
        // if it doesn't exist it will return NULL
        ptrInteger = pNode->GetAlias();
    }
    else
    {
        // Just convert the feature to IInteger.
        // if this isn't possible it will be NULL
        ptrInteger = pNode;
    }

    // We must check IsValid(), since the smart pointer classes will throw an exception
    // if you try to extract a NULL pointer.
    return (ptrInteger.IsValid()) ? (GenApi::IInteger*)ptrInteger : NULL;
}

// Called when a node was possibly changed
void CMCADoc::OnNodeChanged(GenApi::INode* pNode)
{
    if (pNode == NULL)
    {
        return;
    }

    // Uncomment the following line if you want to see which nodes are getting callbacks.
    //TRACE(_T("Node changed: %s\n"), (LPCWSTR)CUtf82W(pNode->GetName().c_str()));

    // Tell the document that some camera features must be updated.
    CWnd* pWnd = AfxGetApp()->GetMainWnd();
    // When the application shuts down the windows may already be gone.
    if (pWnd != NULL)
    {
        // You must use PostMessage here to separate the grab thread from the GUI thread.
        pWnd->PostMessage( WM_COMMAND, MAKEWPARAM( ID_UPDATE_NODES, 0 ), 0 );
    }
    
}

// CMCADoc serialization
void CMCADoc::Serialize(CArchive& ar)
{
    ASSERT(FALSE && "serialization is not supported");
}

// CMCADoc diagnostics
#ifdef _DEBUG
void CMCADoc::AssertValid() const
{
    CDocument::AssertValid();
}


void CMCADoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

// This will be called by MFC before a new document is created
// or when the application is shutting down.
// Perform all cleanup here.
void CMCADoc::DeleteContents()
{
    // Make sure the device is not grabbing.
    OnStopGrab();

    // Free the grab result, if present.
    m_bitmapImage.Release();
    m_ptrGrabResult.Release();

    // Perform cleanup.
    if (m_camera->IsPylonDeviceAttached())
    {
        try
        {
            // Deregister the node callbacks.
            if (m_hExposureTime)
            {
                GenApi::Deregister( m_hExposureTime );
                m_hExposureTime = NULL;
            }
			if (m_hFrameRate)
			{
				GenApi::Deregister(m_hFrameRate);
				m_hFrameRate = NULL;
			}
			if (m_hHeight)
			{
				GenApi::Deregister(m_hHeight);
				m_hHeight = NULL;
			}
			if (m_hWidth)
			{
				GenApi::Deregister(m_hWidth);
				m_hWidth = NULL;
			}
            if (m_hGain)
            {
                GenApi::Deregister( m_hGain );
                m_hGain = NULL;
            }
            if (m_hPixelFormat)
            {
                GenApi::Deregister( m_hPixelFormat );
                m_hPixelFormat = NULL;
            }
            if (m_hTestImage)
            {
                GenApi::Deregister( m_hTestImage );
                m_hTestImage = NULL;
            }

            // Clear the pointer to the features.
            m_ptrExposureTime = NULL;
			m_ptrHeight = NULL;
			m_ptrWidth = NULL;
			m_ptrFrameRate = NULL;
            m_ptrGain = NULL;
            m_ptrTestImage = NULL;
            m_ptrPixelFormat = NULL;

            // Close camera.
            // This will also stop the grab.
            m_camera->Close();

            // Free the camera.
            // This will also stop the grab and close the camera.
            m_camera->DestroyDevice();

            // Tell all the views that there is no camera anymore.
            UpdateAllViews(NULL, UpdateHint_All);
        }
        catch (const Pylon::GenericException& e)
        {
            TRACE(_T("Error during cleanup: %s"), (LPCWSTR)CUtf82W(e.what()) );
            UNUSED(e);
        }
    }

    // Call the base class.
    CDocument::DeleteContents();
}

// Enumerate devices.
void CMCADoc::OnViewRefresh()
{
    try
    {
        // Refresh the list of all attached cameras.
        theApp.EnumerateDevices();

        // Update the GUI.

        // Always update the device list and the image
        EUpdateHint hint = EUpdateHint(UpdateHint_DeviceList | UpdateHint_Image);

        if (m_camera->IsPylonDeviceAttached())
        {
            // in this SDI application the document will be reused.
            // We need to update the window title, in case the camera has been removed.
            hint = EUpdateHint(hint | UpdateHint_Feature);
        }
        else
        {
            // in this SDI application the document will be reused.
            // We need to update the window title, in case the camera has been removed.
            CString s;
            s.LoadString(AFX_IDS_UNTITLED);
            SetTitle(s);
        }

        UpdateAllViews(NULL, hint);
    }
    catch (const Pylon::GenericException& e)
    {
        TRACE(_T("Error during Refresh: %s"), (LPCWSTR)CUtf82W(e.what()));
        UNUSED(e);
    }
}

// Pylon::CImageEventHandler functions
void CMCADoc::OnImagesSkipped(Pylon::CInstantCamera& camera, size_t countOfSkippedImages)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
    ++m_cntSkippedImages;
}


void CMCADoc::OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult)
{
    // NOTE: 
    // This function is called from the CInstantCamera grab thread.
    // You shouldn't perform lengthy operations here.
    // Also, you shouldn't access any UI objects, since we are not in the GUI thread.
    // We just store the grab result in the document and post a message to the CMDIFrameWndEx
    // to notify it of the new result. In response to this message, we will update the GUI.

    // The following line is commented out as this function will be called very often
    // filling up the debug output.
    //TRACE(_T("%s\n"), __FUNCTIONW__);

	++m_cntGrabbedImages;

	TRACE(_T("%s: Camera #%d - %d image(s) grabbed, %d dropped\n"), __FUNCTIONW__, m_id, m_cntGrabbedImages, m_cntDroppedImages);

    // The m_ptrGrabResult will be accessed from different threads,
    // so we need to protect it with the m_MemberLock.
    CSingleLock lock(&m_MemberLock, TRUE);

    // When overwriting the current CGrabResultPtr, the old result will automatically be
    // released and reused by CInstantCamera.
    m_ptrGrabResult = grabResult;

	if (m_ptrGrabResult.IsValid() && m_ptrGrabResult->GrabSucceeded())
	{
		m_buffer.push_back(std::unique_ptr<CImageResult>(new CImageResult(m_ptrGrabResult->GetBuffer(), m_ptrGrabResult->GetImageSize())));

		if (m_buffer.size() > m_bufferSize) {
			std::unique_ptr<CImageResult>& oldElem = m_buffer.front();
			oldElem.reset();
			m_buffer.pop_front();
			++m_cntDroppedImages;
		}
	}

    lock.Unlock();

    // Tell the document that there is a new image available so it can update the image window.
	// skip the 4 frames
	if (1 == (m_cntGrabbedImages % 5)) {
		CWnd* pWnd = AfxGetApp()->GetMainWnd();
		ASSERT(pWnd != NULL);
		if (pWnd != NULL)
		{
			// You must use PostMessage here to separate the grab thread from the GUI thread.
			if (m_id == 0)
				pWnd->PostMessage(WM_COMMAND, MAKEWPARAM(ID_NEW_GRABRESULT_CAMERA1, 0), 0);
			else if (m_id == 1)
				pWnd->PostMessage(WM_COMMAND, MAKEWPARAM(ID_NEW_GRABRESULT_CAMERA2, 0), 0);
			else
				pWnd->PostMessage(WM_COMMAND, MAKEWPARAM(ID_NEW_GRABRESULT, 0), 0);
		}
	}
	// you cant do that, this is from pylon thread
	// OnNewGrabresult();
}

// Pylon::CConfigurationEventHandler functions
void CMCADoc::OnAttach(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnAttached(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnDetach(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnDetached(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnDestroy(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnDestroyed(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnOpen(Pylon::CInstantCamera& camera)
{
    Pylon::String_t strFriendlyName = camera.GetDeviceInfo().GetFriendlyName();
    TRACE(_T("%s - '%s'\n"), __FUNCTIONT__, (LPCWSTR)CUtf82W(strFriendlyName.c_str()));
}


void CMCADoc::OnOpened(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnClose(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnClosed(Pylon::CInstantCamera& camera)
{
    Pylon::String_t strFriendlyName = camera.GetDeviceInfo().GetFriendlyName();
    TRACE(_T("%s - '%s'\n"), __FUNCTIONT__, (LPCWSTR)CUtf82W(strFriendlyName.c_str()));
}


void CMCADoc::OnGrabStart(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
    
    // Reset statistics.
    m_cntGrabbedImages = 0;
    m_cntSkippedImages = 0;
    m_cntGrabErrors = 0;
	m_buffer.clear();
}


void CMCADoc::OnGrabStarted(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnGrabStop(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnGrabStopped(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s Grabbed: %I64u; Errors: %I64u\n"), __FUNCTIONW__, m_cntGrabbedImages, m_cntGrabErrors);

    // Deregister all configurations.
    m_camera->DeregisterConfiguration(&m_singleConfiguration);
    m_camera->DeregisterConfiguration(&m_continousConfiguration);
}


void CMCADoc::OnGrabError(Pylon::CInstantCamera& camera, const char* errorMessage)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);
}


void CMCADoc::OnCameraDeviceRemoved(Pylon::CInstantCamera& camera)
{
    TRACE(_T("%s\n"), __FUNCTIONW__);

    theApp.SetDeviceFullName(NULL);

    try
    {
        // Mark everything as invalid.
        CSingleLock lock(&m_MemberLock, TRUE);
        m_ptrGrabResult.Release();
        m_bitmapImage.Release();
        m_camera->DestroyDevice();

        // Tell the document the camera and the image are gone and let it update the GUI.
        CWnd* pWnd = AfxGetApp()->GetMainWnd();
        ASSERT(pWnd != NULL);
        if (pWnd != NULL)
        {
            // Do a refresh to update the list of devices and the GUI.
            // You must use PostMessage here to separate the grab thread from the GUI thread.
            pWnd->PostMessage(WM_COMMAND, MAKEWPARAM(ID_VIEW_REFRESH, 0), 0);
        }
    }
    catch (Pylon::GenericException)
    {
    }
}

void CMCADoc::UpdateTitle()
{
	CString cameraId;
	CString csTitle;

	if (m_id > -1) {
		cameraId.Format(_T("Camera #%d - "), m_id + 1);
		csTitle = cameraId + m_strPathName;
		SetTitle(csTitle);
	}
	else
		SetTitle(m_strPathName);
}

void CMCADoc::UpdateSettingsDisplay()
{
	UpdateAllViews(NULL, UpdateHint_Feature);
}

BOOL CMCADoc::IsCameraIdle()
{
	return m_camera->IsOpen() && !m_camera->IsGrabbing();
}

BOOL CMCADoc::IsCameraInUse()
{
	return m_camera->IsGrabbing();
}

BOOL CMCADoc::HasImage()
{
	return m_ptrGrabResult.IsValid();
}

CBaslerUsbInstantCamera * CMCADoc::GetUsbCameraPtr()
{
	if (m_camera->IsUsb())
		return dynamic_cast<CBaslerUsbInstantCamera*>(m_camera.get());
	return nullptr;
}


// Called from the GUI thread when there is a new grab result.
// You should update the window displaying the image.
void CMCADoc::OnNewGrabresult()
{
    // Hold a reference to the result to make sure the grab result
    // won't be deleted while we're in this function.
	//Pylon::CGrabResultPtr ptr = m_buffer.back(); // GetGrabResultPtr();
	Pylon::CGrabResultPtr ptr = GetGrabResultPtr();

    // First check whether the smart pointer is valid.
    // Then call GrabSucceeded on the CGrabResultData which the smart pointer references.
    if (ptr.IsValid() && ptr->GrabSucceeded())
    {
		// m_buffer.push_back(ptr);

        // This is where you would do image processing
        // and other tasks.
        // Attention: If you perform lengthy operations, the GUI may become
        // unresponsive as the application doesn't process messages.

        // Convert the grab result to a dib so we can display it on the screen.
        m_bitmapImage.CopyImage(ptr);

		//TRACE(_T("%s: %d image(s) received\n"), __FUNCTIONW__, m_cntGrabbedImages);
    }
    else
    {
        ++m_cntGrabErrors;
        // If the grab result is invalid, we also mark the bitmap as invalid.
        m_bitmapImage.Release();
    }

    UpdateAllViews(NULL, UpdateHint_Image);
}


const Pylon::CGrabResultPtr CMCADoc::GetGrabResultPtr() const
{
    // We must protect this member as it will be accessed from the grab thread and the GUI thread.
    CSingleLock lock(&m_MemberLock, TRUE);
    return m_ptrGrabResult;
}


const Pylon::CPylonBitmapImage& CMCADoc::GetBitmapImage() const
{
    // No need to protect this member as it will only be accessed from the GUI thread.
    return m_bitmapImage;
}


GenApi::IInteger* CMCADoc::GetExposureTime()
{
    // GenICam smart pointers will throw an exception if you try to access a NULL pointer.
    return (m_ptrExposureTime.IsValid()) ? (GenApi::IInteger*)m_ptrExposureTime : NULL;
}

GenApi::IInteger* CMCADoc::GetFrameRate()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrFrameRate.IsValid()) ? (GenApi::IInteger*)m_ptrFrameRate : NULL;
}

GenApi::IInteger* CMCADoc::GetHeight()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrHeight.IsValid()) ? (GenApi::IInteger*)m_ptrHeight : NULL;
}

GenApi::IInteger* CMCADoc::GetWidth()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrWidth.IsValid()) ? (GenApi::IInteger*)m_ptrWidth : NULL;
}


GenApi::IInteger* CMCADoc::GetGain()
{
    // GenICam smart pointers will throw an exception if you try to access a NULL pointer.
    return (m_ptrGain.IsValid()) ? (GenApi::IInteger*)m_ptrGain : NULL;
}

UINT CMCADoc::GetExposureTimeValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrExposureTime.IsValid()) ? m_ptrExposureTime->GetValue() : 0;
}

int CMCADoc::GetFrameRateValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	if (!m_cameraReady)
		return -1;
	
	CBaslerUsbInstantCamera *ptrUsbCamera = GetUsbCameraPtr();
	return nullptr != ptrUsbCamera ? ptrUsbCamera->AcquisitionFrameRate.GetValue() : -1;
}

int CMCADoc::GetResultingFrValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	if (!m_cameraReady)
		return -1;

	CBaslerUsbInstantCamera *ptrUsbCamera = GetUsbCameraPtr();
	if (nullptr == ptrUsbCamera)
		return  -1;

	if (0 == m_id) {
		m_fps = ptrUsbCamera->ResultingFrameRate.GetValue();
		return m_fps;
	}
	return ptrUsbCamera->ResultingFrameRate.GetValue();
}

UINT CMCADoc::GetHeightValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrHeight.IsValid()) ? m_ptrHeight->GetValue() : 0;
}

UINT CMCADoc::GetWidthValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrWidth.IsValid()) ? m_ptrWidth->GetValue() : 0;
}


UINT CMCADoc::GetGainValue()
{
	// GenICam smart pointers will throw an exception if you try to access a NULL pointer.
	return (m_ptrGain.IsValid()) ? m_ptrGain->GetValue() : 0;
}

void CMCADoc::SetFrameRateValue(UINT fr)
{
	if (m_camera->IsUsb()) {
		CBaslerUsbInstantCamera* ptrUsbCamera = GetUsbCameraPtr();
		if (nullptr != ptrUsbCamera)
			ptrUsbCamera->AcquisitionFrameRate.SetValue(fr);
	}
}

void CMCADoc::SaveVideo(CString path, CString timestamp)
{
	UINT duration = CMCADoc::GetDuration();
	UINT fps = CMCADoc::GetFPS();
	UINT totalFramesNumber = duration * fps;
	int size = totalFramesNumber > m_buffer.size() ? m_buffer.size() : totalFramesNumber;

	TRACE(_T("%s: Camera #%d, buffer size - %d, saving frames - %d\n"), __FUNCTIONW__, m_id, m_buffer.size(), size);

	if (size > 0) {
		// Create a video writer object.
		CVideoWriter videoWriter;

		// The frame rate used for playing the video (playback frame rate).
		const int cFramesPerSecond = 25;
		// The quality used for compressing the video.
		const uint32_t cQuality = 100;

		// Map the pixelType
		CEnumerationPtr pixelFormat = GetPixelFormat();
		CPixelTypeMapper pixelTypeMapper(pixelFormat);
		EPixelType pixelType = pixelTypeMapper.GetPylonPixelTypeFromNodeValue(pixelFormat->GetIntValue());
		// Open the video writer.
				// Set parameters before opening the video writer.
		uint32_t width = (uint32_t)GetWidth()->GetValue();
		uint32_t height = (uint32_t)GetHeight()->GetValue();
		videoWriter.SetParameter(
			width,
			height,
			pixelType,
			cFramesPerSecond,
			cQuality);

		CString cameraId;
		CString csTitle;

		if (m_id > -1)
			cameraId.Format(_T("Camera%d-"), m_id + 1);
		else
			cameraId = CString("Camera-");

		csTitle = path + CString("\\") + cameraId + timestamp + CString(".mp4");
		//const char* cstr = (LPCTSTR) csTitle;
		CT2A ascii(csTitle);
		videoWriter.Open(ascii.m_psz);

		try {
			std::list<std::unique_ptr<CImageResult> >::iterator it = m_buffer.end();
			while (size-- > 0)
				--it;
			while (it != m_buffer.end()) {
				std::unique_ptr<CImageResult>& ptrResult = *it;
				videoWriter.Add(ptrResult->GetBuffer(), ptrResult->GetImageSize(), pixelType, width, height, 0, ImageOrientation_TopDown);
				++it;
			}
			videoWriter.Close();
			m_buffer.clear();
		}
		catch (const Pylon::GenericException &e)
		{
			// Error handling.
			TRACE("Error in saving video");
		}
	}
}


GenApi::IEnumeration* CMCADoc::GetTestImage()
{
    // GenICam smart pointers will throw an exception if you try to access a NULL pointer.
    return (m_ptrTestImage.IsValid()) ? (GenApi::IEnumeration*)m_ptrTestImage : NULL;
}


GenApi::IEnumeration* CMCADoc::GetPixelFormat()
{
    // GenICam smart pointers will throw an exception if you try to access a NULL pointer.
    return  (m_ptrPixelFormat.IsValid()) ? (GenApi::IEnumeration*)m_ptrPixelFormat : NULL;
}


// The user clicked on single grab.
void CMCADoc::OnGrabOne()
{
    // Camera may have been disconnected.
    if (!m_camera->IsOpen() || m_camera->IsGrabbing())
    {
        return;
    }

    // Since we may switch between single and continuous shot, we must configure the camera accordingly.
    // The predefined configurations are only executed once when the camera is opened.
    // To be able to use them in our use case, we just call them explicitly to apply the configuration.
    m_singleConfiguration.OnOpened(*m_camera.get());

    // Grab one image.
    // When the image is received, pylon will call out the OnImageGrab() handler.
    m_camera->StartGrabbing(1, Pylon::GrabStrategy_OneByOne, Pylon::GrabLoop_ProvidedByInstantCamera);
}


void CMCADoc::OnUpdateGrabOne(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_camera->IsOpen() && !m_camera->IsGrabbing());
}


void CMCADoc::OnUpdateStartGrabbing(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_camera->IsOpen() && !m_camera->IsGrabbing());
}


// The user clicked on continuous grab.
void CMCADoc::OnStartGrabbing()
{
    // Camera may have been disconnected.
    if (!m_camera->IsOpen() || m_camera->IsGrabbing())
    {
        return;
    }

    // Since we may switch between single and continuous shot, we must configure the camera accordingly.
    // The predefined configurations are only executed once when the camera is opened.
    // To be able to use them in our use case, we just call them explicitly to apply the configuration.
    m_continousConfiguration.OnOpened(*m_camera.get());

    // Start grabbing until StopGrabbing() is called.
    m_camera->StartGrabbing(Pylon::GrabStrategy_OneByOne, Pylon::GrabLoop_ProvidedByInstantCamera);
	// m_camera->StartGrabbing(m_bufferSize);
	// retrieve the result in main thread
}


// The user clicked on stop grab.
void CMCADoc::OnStopGrab()
{
    // Camera may have been disconnected.
    if (NULL != m_camera && m_camera->IsGrabbing())
    {
        m_camera->StopGrabbing();
    }
}


void CMCADoc::OnUpdateStopGrab(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(NULL != m_camera ? m_camera->IsOpen() && m_camera->IsGrabbing() : FALSE);
}

// We overwrite the OnOpenDocument handler to open a camera device.
// In our case, lpszPathName is the full name of the device.
BOOL CMCADoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    // Note: lpszPathName is the full name of the camera.

    // Make sure everything is closed.
    DeleteContents();

	try
	{
		CString deviceName = CString(lpszPathName).MakeLower();
		BOOL isUSBCamera = FALSE;

		isUSBCamera = m_camera->IsUsb();

		//if (!isUSBCamera) {
		//	if (deviceName.Find(_T("ace")) == 0 || deviceName.Find(_T("daa")) == 0 || deviceName.Find(_T("pua")) == 0)
		//		isUSBCamera = TRUE;
		//}

		CBaslerUsbInstantCamera *ptrUsbCamera = nullptr;
		CInstantCamera* ptrCamera = (isUSBCamera ? (ptrUsbCamera = new CBaslerUsbInstantCamera()) : new CInstantCamera());
		m_camera = std::unique_ptr<CInstantCamera>(ptrCamera);

		ASSERT(!m_camera->IsPylonDeviceAttached());

		// Add the AutoPacketSizeConfiguration and let pylon delete it when not needed anymore.
		m_camera->RegisterConfiguration(new CAutoPacketSizeConfiguration(), Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);

		// Create the device and attach it to CInstantCamera.
		// Let CInstantCamera take care of destroying the device.
		const Pylon::String_t strDeviceFullName = GetString_t(lpszPathName);
		Pylon::IPylonDevice* pDevice = Pylon::CTlFactory::GetInstance().CreateDevice(strDeviceFullName);
		m_camera->Attach(pDevice, Pylon::Cleanup_Delete);

		// Open camera.
		m_camera->Open();

		// Get the Exposure Time feature.
		// On GigE cameras, the feature is named 'ExposureTimeRaw'.
		// On USB cameras, it is named 'ExposureTime'.
		m_ptrExposureTime = GetIntegerFeature(m_camera->GetNodeMap().GetNode("ExposureTime"));
		if (!m_ptrExposureTime.IsValid())
		{
			m_ptrExposureTime = GetIntegerFeature(m_camera->GetNodeMap().GetNode("ExposureTimeRaw"));
		}
		if (m_ptrExposureTime.IsValid())
		{
			// Add a callback that triggers the update.
			m_hExposureTime = GenApi::Register(m_ptrExposureTime->GetNode(), *this, &CMCADoc::OnNodeChanged);
		}

		//
		m_ptrWidth = GetIntegerFeature(m_camera->GetNodeMap().GetNode("Width"));
		if (!m_ptrWidth.IsValid())
		{
			m_ptrWidth = GetIntegerFeature(m_camera->GetNodeMap().GetNode("WidthRaw"));
		}
		if (m_ptrWidth.IsValid())
		{
			// Add a callback that triggers the update.
			m_hWidth = GenApi::Register(m_ptrWidth->GetNode(), *this, &CMCADoc::OnNodeChanged);
		}

		//
		m_ptrHeight = GetIntegerFeature(m_camera->GetNodeMap().GetNode("Height"));
		if (!m_ptrHeight.IsValid())
		{
			m_ptrHeight = GetIntegerFeature(m_camera->GetNodeMap().GetNode("HeightRaw"));
		}
		if (m_ptrHeight.IsValid())
		{
			// Add a callback that triggers the update.
			m_hHeight = GenApi::Register(m_ptrHeight->GetNode(), *this, &CMCADoc::OnNodeChanged);
		}

		if (nullptr != ptrUsbCamera) {
			if (GenApi::IsWritable(ptrUsbCamera->AcquisitionFrameRateEnable))
			{
				ptrUsbCamera->AcquisitionFrameRateEnable = true;
			}
			else { TRACE(CUtf82W("NOTE: cannot set Acquisition Frame Rate")); }
		
		
			m_ptrFrameRate = GetIntegerFeature(m_camera->GetNodeMap().GetNode("AcquisitionFrameRate"));
			if (!m_ptrFrameRate.IsValid())
			{
				m_ptrFrameRate = GetIntegerFeature(m_camera->GetNodeMap().GetNode("AcquisitionFrameRateAbs"));
			}
			if (!m_ptrFrameRate.IsValid())
			{
				try {
					m_ptrFrameRate = (GenApi::IInteger*)&ptrUsbCamera->AcquisitionFrameRate;
				}
				catch (...) {

				}
			}
			if (m_ptrFrameRate.IsValid())
			{
				// Add a callback that triggers the update.
				m_hFrameRate = GenApi::Register(m_ptrFrameRate->GetNode(), *this, &CMCADoc::OnNodeChanged);
			}
		}
        // Get the Gain feature.
        // On GigE cameras, the feature is named 'GainRaw'.
        // On USB cameras, it is named 'Gain'.
        m_ptrGain = GetIntegerFeature( m_camera->GetNodeMap().GetNode( "Gain" ) );
        if (!m_ptrGain.IsValid())
        {
            m_ptrGain = GetIntegerFeature( m_camera->GetNodeMap().GetNode( "GainRaw" ) );
        }
        if (m_ptrGain.IsValid())
        {   // Add a callback that triggers the update.
            m_hGain = GenApi::Register( m_ptrGain->GetNode(), *this, &CMCADoc::OnNodeChanged );
        }

        // Get the Test Image Selector feature.
        m_ptrTestImage = m_camera->GetNodeMap().GetNode( "TestImageSelector" );
        if (m_ptrTestImage.IsValid())
        {
            // Add a callback that triggers the update.
            m_hTestImage = GenApi::Register(m_ptrTestImage->GetNode(), *this, &CMCADoc::OnNodeChanged);
        }

        // Get the Pixel Format feature.
        m_ptrPixelFormat = m_camera->GetNodeMap().GetNode( "PixelFormat" );
        if (m_ptrPixelFormat.IsValid())
        {
            // Add a callback that triggers the update.
            m_hPixelFormat = GenApi::Register(m_ptrPixelFormat->GetNode(), *this, &CMCADoc::OnNodeChanged);
        }

        // Clear everything.
        CSingleLock lock(&m_MemberLock, TRUE);
        m_ptrGrabResult.Release();
        m_bitmapImage.Release();

        // Mark the document as "not modified".
        SetModifiedFlag(FALSE);

		m_cameraReady = TRUE;
        return TRUE;
    }
    catch (const Pylon::GenericException& e)
    {
        UNUSED(e);
        TRACE(CUtf82W(e.what()));
        
        // Something went wrong. Make sure we don't leave the document object partially initialized.
        DeleteContents();
        
        return FALSE;
    }
}

// In our case, lpszPathName is the full name of the device and not an existing file.
// We provide an override to prevent the MFC implementation returning an error
// because the actual file does not exist.
void CMCADoc::SetPathName(LPCTSTR lpszPathName, BOOL /* bAddToMRU */)
{
	m_strPathName = lpszPathName;
	ASSERT(!m_strPathName.IsEmpty());
	m_bEmbedded = FALSE;
	ASSERT_VALID(this);

	UpdateTitle();
}

BOOL CMCADoc::RegisterListeners()
{
	try
	{
		// Register this object as an image event handler, so we will be notified of new new images
		// See Pylon::CImageEventHandler for details
		m_camera->RegisterImageEventHandler(this, Pylon::RegistrationMode_ReplaceAll, Pylon::Ownership_ExternalOwnership);
		// Register this object as a configuration event handler, so we will be notified of camera state changes.
		// See Pylon::CConfigurationEventHandler for details
		m_camera->RegisterConfiguration(this, Pylon::RegistrationMode_ReplaceAll, Pylon::Ownership_ExternalOwnership);
	}
	catch (const Pylon::GenericException& e)
	{
		TRACE(CUtf82W(e.what()));
		return FALSE;

		UNUSED(e);
	}

	return TRUE;
}

// Save the image to disk.
void CMCADoc::OnFileImageSaveAs()
{
    // Make sure no one overwrites the grab result while we display the file dialog.
    // By holding a reference we make sure the grab result won't be
    // used by the CInstantCamera grab engine while doing a continuous grab.
    Pylon::CGrabResultPtr ptrGrabResult = GetGrabResultPtr();

    // We must have a valid grab result in order to save.
    if (!ptrGrabResult.IsValid())
    {
        return;
    }

    CString strFilter;
    strFilter.LoadString(IDS_FILTER_IMAGETYPES);

    // Show file dialog
    CFileDialog dlg(FALSE, NULL, _T("*.bmp"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, AfxGetMainWnd());

    if (dlg.DoModal() == IDOK)
    {
        try
        {
            // Convert the selected file type to EImageFileFormat enum.
            // Note: nFilterIndex starts at 1 from the first selected type
            Pylon::EImageFileFormat imageFileFormat = static_cast<Pylon::EImageFileFormat>(dlg.m_ofn.nFilterIndex - 1);
            if (imageFileFormat < Pylon::ImageFileFormat_Bmp || imageFileFormat > Pylon::ImageFileFormat_Raw)
            {
                throw Pylon::RuntimeException("Invalid file type", __FILE__, __LINE__);
            }

            // For demonstration purposes, we output a warning if the grab result cannot be saved without conversion.
            if (!Pylon::CImagePersistence::CanSaveWithoutConversion(imageFileFormat, m_ptrGrabResult))
            {
                TRACE(_T("Warning: Image must be converted to be saved."));
            }

            CString strFullFileName = dlg.GetPathName();

            // Save the file to disk.
            // The image will be converted automatically if the target format 
            // doesn't support the bit depth or the pixel format of the grab result.
            Pylon::CImagePersistence::Save(imageFileFormat, GetString_t(strFullFileName), ptrGrabResult);
        }
        catch (const Pylon::GenericException& e)
        {
            CString strErrorMessage;
            strErrorMessage.Format(_T("Could not save image!\n\n(%s)"), (LPCWSTR)CUtf82W(e.what()));

            AfxMessageBox(strErrorMessage, MB_OK | MB_ICONSTOP);
        }
    }
}


void CMCADoc::OnUpdateFileImageSaveAs(CCmdUI *pCmdUI)
{
    // We can only save if we have a valid grab result.
    pCmdUI->Enable(HasImage());
}

// Called when a camera feature changes its properties.
void CMCADoc::OnUpdateNodes()
{
    // Check the camera. It may have been removed.
    if (m_camera->IsPylonDeviceAttached())
    {
		UpdateSettingsDisplay();
    }
}
