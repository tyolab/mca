#include "stdafx.h"
#include "CameraInfo.h"

#include "MCADoc.h"
#include "ConfigView.h"

CCameraInfo::CCameraInfo():
	m_pConfigView(NULL)
	, m_pCameraDoc(NULL)
{
	m_id = -1;

}

CCameraInfo::~CCameraInfo()
{
}

void CCameraInfo::GrabOne()
{
	if (NULL != m_pCameraDoc)
		m_pCameraDoc->OnGrabOne();
}

void CCameraInfo::GrabMore(uint64_t timestamp)
{
	if (NULL != m_pCameraDoc)
		m_pCameraDoc->StartGrabbing(timestamp);
}

void CCameraInfo::StopGrabbing()
{
	if (NULL != m_pCameraDoc) {
		m_pCameraDoc->OnStopGrab();
	}
}

void CCameraInfo::SaveVideo(CString path, CString timestamp, UINT frameCount)
{
	if (NULL != m_pCameraDoc) {
		m_pCameraDoc->SaveVideo(path, timestamp, frameCount);
	}
}

void CCameraInfo::setMCADoc(CMCADoc* cameraDoc)
{
	m_pCameraDoc = cameraDoc;
}

BOOL CCameraInfo::IsCameraInUse()
{
	if (NULL != m_pCameraDoc)
		return m_pCameraDoc->IsCameraInUse();
	return FALSE;
}

BOOL CCameraInfo::IsCameraIdle()
{
	if (NULL != m_pCameraDoc)
		return m_pCameraDoc->IsCameraIdle();
	return FALSE;
}

BOOL CCameraInfo::HasImage()
{
	if (NULL != m_pCameraDoc)
		return m_pCameraDoc->HasImage();
	return FALSE;
}
