#pragma once

class CMCADoc;
class CConfigView;

class CCameraInfo
{
public:
	CString m_strDeviceFullName;
	CConfigView *m_pConfigView;
	CMCADoc *m_pCameraDoc;

private:
	int m_id;

public:
	CCameraInfo();
	virtual ~CCameraInfo();

	void GrabOne();
	void GrabMore(uint64_t timestamp);
	void StopGrabbing();

	void SaveVideo(CString paht, CString timestamp, UINT frameCount);

	void SetID(int id) {
		m_id = id;
	}

	int GetID() {
		return m_id;
	}

	void setMCADoc(CMCADoc* cameraDoc);

	BOOL IsCameraInUse();
	BOOL IsCameraIdle();
	BOOL HasImage();
};

