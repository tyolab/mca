#include "stdafx.h"
#include "CameraInfo.h"

#include "MCADoc.h"
#include "ConfigView.h"

CCameraInfo::CCameraInfo()
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

void CCameraInfo::GrabMore()
{
	if (NULL != m_pCameraDoc)
		m_pCameraDoc->OnStartGrabbing();
}

void CCameraInfo::StopGrap()
{
	if (NULL != m_pCameraDoc)
		m_pCameraDoc->OnStopGrab();
}

void CCameraInfo::setMCADoc(CMCADoc* cameraDoc)
{
	m_pCameraDoc = cameraDoc;
}
