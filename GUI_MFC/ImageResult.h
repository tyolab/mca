#pragma once

class CImageResult
{
private:
	void* m_buffer;
	UINT m_bufferSize;

public:
	CImageResult(const void *buffer, UINT size);
	~CImageResult();

	const void* GetBuffer() {
		return m_buffer;
	}

	const UINT GetImageSize() {
		return m_bufferSize;
	}
};

