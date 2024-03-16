/*
	Copyright (c) 2024 Thomas Kuhn

	Redistribution and use in source and binary forms, with or without modification, are permitted provided
	that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
	promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
	OF SUCH DAMAGE.
*/
#include "tools.h"

// Convert a string to a wide string
std::wstring Tools::stringToWstring(const std::string& str, int codepage)
{
	// Get the length of the wide string
	int len = MultiByteToWideChar(codepage, 0, str.c_str(), -1, NULL, 0);
	// If the length is greater than 0
	if (len > 0)
	{
		// Create a wide string and resize it to the correct length
		std::wstring wstr;
		wstr.resize(len);
		// Convert the string to a wide string
		MultiByteToWideChar(codepage, 0, str.c_str(), -1, &wstr[0], len);
		// Return the wide string
		return wstr;
	}
	// If the length is not greater than 0, return an empty wide string
	return std::wstring();
}

// Convert a wide string to a string
std::string Tools::wstringToString(const std::wstring& wstr, int codepage)
{
	// Get the length of the string
	int len = WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	// If the length is greater than 0
	if (len > 0)
	{
		// Create a string and resize it to the correct length
		std::string str;
		str.resize(len);
		// Convert the wide string to a string
		WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);
		// Return the string
		return str;
	}
	// If the length is not greater than 0, return an empty string
	return std::string();
}
