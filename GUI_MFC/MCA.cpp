/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    
    Defines the class behaviors for the application.
*/
#include "stdafx.h"
#include "MCA.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "MCADoc.h"
#include "ConfigView.h"
#include "MCAView.h"

#include <windows.h>
#include <shlobj.h>

#include <ctime>

#include <pylon/PylonIncludes.h>

#pragma comment(lib, "shell32.lib")

#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMCAApp
BEGIN_MESSAGE_MAP(CMCAApp, CWinAppEx)
	//
	ON_COMMAND(ID_CAMERA_GRABONE, &CMCAApp::OnGrabOne)
	ON_COMMAND(ID_CAMERA_STARTGRABBING, &CMCAApp::OnStartGrabbing)
	ON_COMMAND(ID_CAMERA_STOPGRAB, &CMCAApp::OnStopGrab)

	ON_UPDATE_COMMAND_UI(ID_CAMERA_GRABONE, &CMCAApp::OnUpdateGrabOne)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_STARTGRABBING, &CMCAApp::OnUpdateStartGrabbing)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_STOPGRAB, &CMCAApp::OnUpdateStopGrab)
	ON_UPDATE_COMMAND_UI(ID_FILE_IMAGE_SAVE_AS, &CMCAApp::OnUpdateFileImageSaveAs)

	ON_COMMAND(ID_NEW_GRABRESULT_CAMERA1, &CMCAApp::OnNewGrabresultCamera1)
	ON_COMMAND(ID_NEW_GRABRESULT_CAMERA2, &CMCAApp::OnNewGrabresultCamera2)
	//
    ON_COMMAND(ID_APP_ABOUT, &CMCAApp::OnAppAbout)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
    ON_COMMAND(ID_OPEN_CAMERA, &CMCAApp::OnOpenCamera)
END_MESSAGE_MAP()

// CMCAApp construction
CMCAApp::CMCAApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
	m_cameraInfo1.SetID(0);
	m_cameraInfo2.SetID(1);
}

// The one and only CMCAApp object
CMCAApp theApp;

// CMCAApp initialization
BOOL CMCAApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinAppEx::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("tyolab\\mca"));
    LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);


    // Before using any pylon methods, the pylon runtime must be initialized.
    Pylon::PylonInitialize();

    // Get list of all attached devices.
    EnumerateDevices();
    
    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views
    // CSingleDocTemplate* pDocTemplate;
    // pDocTemplate = new CSingleDocTemplate(
    //     IDR_MAINFRAME,
    //     RUNTIME_CLASS(CMCADoc),
    //     RUNTIME_CLASS(CMainFrame),       // main SDI frame window
    //     RUNTIME_CLASS(CConfigView));
    // if (!pDocTemplate)
    //     return FALSE;
    // AddDocTemplate(pDocTemplate);

    EnableTaskbarInteraction();

    CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
        IDR_MAINFRAME,
		RUNTIME_CLASS(CMCADoc),
        RUNTIME_CLASS(CChildFrame),       // main SDI frame window
        RUNTIME_CLASS(CMCAView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

    //create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

	// disable new file
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)   // actually none
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

    // Dispatch commands specified on the command line.  Will return FALSE if
    // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    // The one and only window has been initialized, so show and update it
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand

	// Initialize output folder
	wchar_t videosFolder[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT, videosFolder);

	if (result != S_OK)
		TRACE( "Error in getting the videos folder \n");

	m_strOutputFolder = videosFolder + CString("\\MCA");
	if (CreateDirectory(m_strOutputFolder, NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		//nothing
	}
	else
	{
		// Failed to create directory.
		TRACE("Error in creating the videos output folder \n");
	}

    return TRUE;
}


int CMCAApp::ExitInstance()
{
    // Releases all pylon resources.
    Pylon::PylonTerminate();

    return CWinAppEx::ExitInstance();
}

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMCAApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CMCAApp::OnGrabOne()
{
	m_cameraInfo1.GrabOne();
	m_cameraInfo2.GrabOne();
}

void CMCAApp::OnStartGrabbing()
{
	if (!Pylon::CVideoWriter::IsSupported())
	{
		CString strErrorMessage;
		strErrorMessage.Format(_T("Video recording is not supported at the moment. Please install the pylon Supplementary Package for MPEG-4 which is available on the Basler website.\"%s\""));
		AfxMessageBox(strErrorMessage);
		return;
	}

	m_cameraInfo1.GrabMore();
	m_cameraInfo2.GrabMore();
}

void CMCAApp::OnStopGrab()
{
	m_cameraInfo1.StopGrabbing();
	m_cameraInfo2.StopGrabbing();

	CWaitCursor wait;
	// saving video now
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", timeinfo);

	CString timestamp = CString(buffer);
	m_cameraInfo1.SaveVideo(m_strOutputFolder, timestamp);
	m_cameraInfo2.SaveVideo(m_strOutputFolder, timestamp);
}

void CMCAApp::OnNewGrabresultCamera1()
{
	if (NULL != m_cameraInfo1.m_pCameraDoc)
		m_cameraInfo1.m_pCameraDoc->OnNewGrabresult();
}

void CMCAApp::OnNewGrabresultCamera2()
{
	if (NULL != m_cameraInfo1.m_pCameraDoc)
		m_cameraInfo2.m_pCameraDoc->OnNewGrabresult();
}

// CMCAApp message handlers
const Pylon::DeviceInfoList_t& CMCAApp::GetDeviceInfoList() const
{
    return m_devices;
}

// Get list of all attached devices.
int CMCAApp::EnumerateDevices()
{
    Pylon::DeviceInfoList_t devices;
    try
    {
        // Get the transport layer factory.
        Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();

        // Get all attached cameras.
        TlFactory.EnumerateDevices(devices);
    }
    catch (const Pylon::GenericException& e)
    {
        UNUSED(e);
        devices.clear();

        TRACE(CUtf82W(e.what()));
    }

    m_devices = devices;

    // When calling this function, make sure to update the device list control,
    // because its items store pointers to elements in the m_devices list.
    return (int)m_devices.size();
}

// Will be called to pass the full name of the camera selected in the device list control.
CString CMCAApp::SetDeviceFullName(LPCTSTR lpszFullDeviceName, int id)
{
    CString oldValue = m_strDeviceFullName;
    m_strDeviceFullName = lpszFullDeviceName;
	m_currentDeviceID = id;

	if (id == 0)
		m_cameraInfo1.m_strDeviceFullName = lpszFullDeviceName;
	else if (id == 1)
		m_cameraInfo2.m_strDeviceFullName = lpszFullDeviceName;
		
    return oldValue;
}

void CMCAApp::GetAllDocuments(DocsPtrList & listDocs)
{
	// clear the list
	listDocs.RemoveAll();
	// loop through application's document templates
	POSITION posDocTemplate = GetFirstDocTemplatePosition();
	while (NULL != posDocTemplate)
	{
		CDocTemplate* pDocTemplate = GetNextDocTemplate(posDocTemplate);

		// get each document open in given document template
		POSITION posDoc = pDocTemplate->GetFirstDocPosition();
		while (NULL != posDoc)
		{
			CDocument* pDoc = pDocTemplate->GetNextDoc(posDoc);
			listDocs.AddTail(pDoc); // add document to list
		}
	}
}

void CMCAApp::OnOpenCamera()
{
    // Do nothing if there is no camera.
    if (m_strDeviceFullName.IsEmpty())
    {
        return;
    }

	// Get the config view
	CMainFrame* mainFrm = reinterpret_cast<CMainFrame*>(GetMainWnd());
	CConfigView* configView = NULL;
	CCameraInfo* pCameraInfo = NULL;

	if (m_currentDeviceID == 0) {
		configView = mainFrm->getConfigViewCamera1();
		pCameraInfo = &m_cameraInfo1;

		if (NULL != m_cameraInfo2.m_pConfigView) {
			configView->SetPartnerView(m_cameraInfo2.m_pConfigView);
			m_cameraInfo2.m_pConfigView->SetPartnerView(configView);
		}
	}
	else if (m_currentDeviceID == 1) {
		configView = mainFrm->getConfigViewCamera2();
		pCameraInfo = &m_cameraInfo2;

		if (NULL != m_cameraInfo1.m_pConfigView) {
			configView->SetPartnerView(m_cameraInfo1.m_pConfigView);
			m_cameraInfo1.m_pConfigView->SetPartnerView(configView);
		}
	}

	CDocument* pNewDoc = NULL;
	if (NULL == pCameraInfo->m_pCameraDoc) {

		// We only have one doc template. Get a pointer to it.
		POSITION pos = GetFirstDocTemplatePosition();
		CDocTemplate* pDocTemplate = GetNextDocTemplate(pos);

		// Open the document and use the full name of the device as the filename.
		pNewDoc = pDocTemplate->OpenDocumentFile(m_strDeviceFullName, TRUE);

		if (pNewDoc == NULL)
		{
			CString strErrorMessage;
			strErrorMessage.Format(_T("Could not open camera \"%s\""), m_strDeviceFullName);
			AfxMessageBox(strErrorMessage);
		}
		else {
			if (NULL != configView) {
				pNewDoc->AddView(configView);
				CMCADoc* pDoc = reinterpret_cast<CMCADoc*>(pNewDoc);
				pDoc->SetID(m_currentDeviceID);
				pDoc->RegisterListeners();
				pDoc->UpdateTitle();
				pDoc->OnUpdateNodes();
				//pDoc->SetModifiedFlag(true);
				pCameraInfo->m_pCameraDoc = pDoc;
				pCameraInfo->m_pConfigView = configView;
			}
		}
	}
	
}

void CMCAApp::OnUpdateGrabOne(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_cameraInfo1.IsCameraIdle() || m_cameraInfo2.IsCameraIdle());
}

void CMCAApp::OnUpdateStartGrabbing(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_cameraInfo1.IsCameraIdle() || m_cameraInfo2.IsCameraIdle());
}

void CMCAApp::OnUpdateStopGrab(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_cameraInfo1.IsCameraInUse() || m_cameraInfo2.IsCameraInUse());
}

void CMCAApp::OnUpdateFileImageSaveAs(CCmdUI *pCmdUI)
{
	// We can only save if we have a valid grab result.
	pCmdUI->Enable(m_cameraInfo1.HasImage() || m_cameraInfo2.HasImage());
}