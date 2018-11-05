#include "stdafx.h"
#include "ImageResult.h"

#define _CRT_SECURE_DEPRECATE_MEMORY  
#include <memory.h>  

CImageResult::CImageResult(const void *buffer, UINT size) :
	m_TimeStamp(0)
{
	m_buffer = new uint8_t[m_bufferSize = size];
	memcpy(m_buffer, buffer, size);
}


CImageResult::~CImageResult()
{
	delete[] m_buffer;
}
