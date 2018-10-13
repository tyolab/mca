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

// Hints to pass to UpdateAllViews(), so we can update specific parts of the GUI.
enum EUpdateHint
{
      UpdateHint_All = 0
    , UpdateHint_Image = 1
    , UpdateHint_Feature = 2
    , UpdateHint_DeviceList = 4
};

class CMCADoc :
      public CDocument
    , public Pylon::CImageEventHandler             // Allows you to get informed about received images and grab errors.
    , public Pylon::CConfigurationEventHandler     // Allows you to get informed about device removal.

{
protected: // create from serialization only
    DECLARE_DYNCREATE(CMCADoc)

public:
	CMCADoc();

// Attributes
public:
    const Pylon::CGrabResultPtr GetGrabResultPtr() const;
    const Pylon::CPylonBitmapImage& GetBitmapImage() const;
    GenApi::IInteger* GetExposureTime();
    GenApi::IInteger* GetGain();
    GenApi::IEnumeration* GetTestImage();
    GenApi::IEnumeration* GetPixelFormat();

// Operations
public:

// Overrides
public:
    virtual BOOL OnNewDocument();
    virtual void DeleteContents();
    virtual void Serialize(CArchive& ar);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

// Implementation
public:
    virtual ~CMCADoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    GenApi::IInteger* GetIntegerFeature(GenApi::INode* pNode) const;
    void OnNodeChanged(GenApi::INode*);

// Pylon::CImageEventHandler functions
    virtual void OnImagesSkipped(Pylon::CInstantCamera& camera, size_t countOfSkippedImages);
    virtual void OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult);

// Pylon::CConfigurationEventHandler functions
    virtual void OnAttach(Pylon::CInstantCamera& camera);
    virtual void OnAttached(Pylon::CInstantCamera& camera);
    virtual void OnDetach(Pylon::CInstantCamera& camera);
    virtual void OnDetached(Pylon::CInstantCamera& camera);
    virtual void OnDestroy(Pylon::CInstantCamera& camera);
    virtual void OnDestroyed(Pylon::CInstantCamera& camera);
    virtual void OnOpen(Pylon::CInstantCamera& camera);
    virtual void OnOpened(Pylon::CInstantCamera& camera);
    virtual void OnClose(Pylon::CInstantCamera& camera);
    virtual void OnClosed(Pylon::CInstantCamera& camera);
    virtual void OnGrabStart(Pylon::CInstantCamera& camera);
    virtual void OnGrabStarted(Pylon::CInstantCamera& camera);
    virtual void OnGrabStop(Pylon::CInstantCamera& camera);
    virtual void OnGrabStopped(Pylon::CInstantCamera& camera);
    virtual void OnGrabError(Pylon::CInstantCamera& camera, const char* errorMessage);
    virtual void OnCameraDeviceRemoved(Pylon::CInstantCamera& camera);

protected:
    mutable CCriticalSection m_MemberLock;
    uint64_t m_cntGrabbedImages;
    uint64_t m_cntSkippedImages;
    uint64_t m_cntGrabErrors;

private:
    // The camera
    Pylon::CInstantCamera m_camera;
    // The grab result retrieved from the camera
    Pylon::CGrabResultPtr m_ptrGrabResult;
    // The grab result as a windows DIB to be displayed on the screen
    Pylon::CPylonBitmapImage m_bitmapImage;
    
    Pylon::CAcquireSingleFrameConfiguration m_singleConfiguration;
    Pylon::CAcquireContinuousConfiguration m_continousConfiguration;

    // Smart pointer to camera features
    GenApi::CIntegerPtr m_ptrExposureTime;
    GenApi::CIntegerPtr m_ptrGain;
    GenApi::CEnumerationPtr m_ptrTestImage;
    GenApi::CEnumerationPtr m_ptrPixelFormat;
    // Callback handles
    GenApi::CallbackHandleType m_hExposureTime;
    GenApi::CallbackHandleType m_hGain;
    GenApi::CallbackHandleType m_hPixelFormat;
    GenApi::CallbackHandleType m_hTestImage;

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnViewRefresh();
    afx_msg void OnNewGrabresult();
    afx_msg void OnGrabOne();
    afx_msg void OnStartGrabbing();
    afx_msg void OnStopGrab();
    afx_msg void OnUpdateGrabOne(CCmdUI *pCmdUI);
    afx_msg void OnUpdateStartGrabbing(CCmdUI *pCmdUI);
    afx_msg void OnUpdateStopGrab(CCmdUI *pCmdUI);
    afx_msg void OnFileImageSaveAs();
    afx_msg void OnUpdateFileImageSaveAs(CCmdUI *pCmdUI);
    afx_msg void OnUpdateNodes();
};
