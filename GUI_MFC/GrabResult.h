#pragma once

#include <pylon/PylonIncludes.h>

using namespace Pylon;
using namespace GenApi;

class CGrabResult
{
public:
	time_t m_timestamp;
	CGrabResultPtr m_result;

public:
	CGrabResult(CGrabResultPtr& result);
	~CGrabResult();
};

