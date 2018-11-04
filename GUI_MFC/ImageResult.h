#pragma once

class CImageResult
{
private:
	void* m_buffer;
	UINT m_bufferSize;
	uint64_t m_TimeStamp;

public:
	CImageResult(const void *buffer, UINT size);
	~CImageResult();

	const void* GetBuffer() {
		return m_buffer;
	}

	const UINT GetImageSize() {
		return m_bufferSize;
	}

	void SetTimeStamp(uint64_t timestamp) {
		m_TimeStamp = timestamp;
	}

	uint64_t GetTimeStamp() const
	{
		return m_TimeStamp;
	}
};

