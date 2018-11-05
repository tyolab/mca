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

#include <list>
#include <memory>

#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

#include "ImageResult.h"

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
	GenApi::IInteger* GetFrameRate();
	GenApi::IInteger* GetResultingFr() { return NULL; }
	GenApi::IInteger* GetWidth();
	GenApi::IInteger* GetHeight();
    GenApi::IInteger* GetGain();
    GenApi::IEnumeration* GetTestImage();
    GenApi::IEnumeration* GetPixelFormat();

	std::list<std::unique_ptr<CImageResult> >* GetBufferPtr() {
		return &m_buffer;
	}

	UINT GetExposureTimeValue();
	int GetFrameRateValue();
	int GetResultingFrValue();
	UINT GetWidthValue();
	UINT GetHeightValue();
	UINT GetGainValue();

	void SetFrameRateValue(UINT fr);

	void SaveVideo(CString path, CString timestamp, UINT frameCount);

	static const UINT GetDuration() {
		return m_duration;
	}
	static void SetDuration(UINT duration) {
		if (duration < 1)
			return;
		m_duration = duration;
	}

	static const UINT GetBufferSize() {
		return m_bufferSize;
	}

	static void SetBufferSize(UINT bufferSize) {
		m_bufferSize = bufferSize;
	}

	static const UINT GetFPS() {
		return m_fps;
	}

// Operations
public:

// Overrides
public:
    virtual BOOL OnNewDocument();
    virtual void DeleteContents();
    virtual void Serialize(CArchive& ar);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual BOOL RegisterListeners();

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

	void SetID(int id) {
		m_id = id;
	}

	int GetID() {
		return m_id;
	}

	void UpdateTitle();
	void UpdateSettingsDisplay();
	BOOL IsCameraIdle();
	BOOL IsCameraInUse();
	BOOL HasImage();

	Pylon::CBaslerUsbInstantCamera* GetUsbCameraPtr();

	void StartGrabbing(uint64_t timestamp);
	void Align(uint64_t timestampStart, uint64_t timestamp);

protected:
    mutable CCriticalSection m_MemberLock;
    uint64_t m_cntGrabbedImages;
	uint64_t m_cntDroppedImages;
    uint64_t m_cntSkippedImages;
    uint64_t m_cntGrabErrors;

	static UINT m_duration;
	static UINT m_bufferSize;
	static UINT m_fps; // the real resulting frame rate 
	static UINT m_fpsView; // for updating the view fresh rate, make 10 initially, which means the view get refreshed for continious grabbing for very 10 images 

private:
	int			 m_id;
	CString		 m_title;
	BOOL		 m_cameraReady;
	BOOL		 m_isUsbCamera;

	// Recording timestamp 
	uint64_t	 m_firstTimeStamp;

	// the first frame timestamp from camera
	uint64_t	 m_firstTimeStampCamera;

	// the timestamp for starting recording
	uint64_t	 m_startTimeStamp;

	// frame buffer
	std::list<std::unique_ptr<CImageResult> > m_buffer;

    // The camera
    std::unique_ptr<Pylon::CInstantCamera> m_camera;
    // The grab result retrieved from the camera
    Pylon::CGrabResultPtr m_ptrGrabResult;
    // The grab result as a windows DIB to be displayed on the screen
    Pylon::CPylonBitmapImage m_bitmapImage;
    
    Pylon::CAcquireSingleFrameConfiguration m_singleConfiguration;
    Pylon::CAcquireContinuousConfiguration m_continousConfiguration;

    // Smart pointer to camera features
    GenApi::CIntegerPtr m_ptrExposureTime;
	GenApi::CIntegerPtr m_ptrHeight;
	GenApi::CIntegerPtr m_ptrWidth;
	GenApi::CIntegerPtr m_ptrFrameRate;
    GenApi::CIntegerPtr m_ptrGain;
    GenApi::CEnumerationPtr m_ptrTestImage;
    GenApi::CEnumerationPtr m_ptrPixelFormat;
    // Callback handles
	GenApi::CallbackHandleType m_hWidth;
	GenApi::CallbackHandleType m_hHeight;
    GenApi::CallbackHandleType m_hExposureTime;
    GenApi::CallbackHandleType m_hGain;
	GenApi::CallbackHandleType m_hFrameRate;
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
    afx_msg void OnUpdateFileImageSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnFileImageSaveAs();
    afx_msg void OnUpdateNodes();
};
