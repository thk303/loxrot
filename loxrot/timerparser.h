#pragma once

#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <windows.h>

using namespace std;

namespace timerparser {
	typedef runtime_error timestring_error;
	bool isTimeToRotate(const wstring& rotateTimes);
}
