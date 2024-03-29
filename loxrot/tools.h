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

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
	OF SUCH DAMAGE.
*/
#pragma once
#include <string>
#include <windows.h>

/**
 * \class Tools
 * \brief A class for utility functions.
 */
class Tools
{
	public:
		/**
   * \brief Convert a string to a wide string.
   * \param str The string to convert.
   * \param codepage The code page to use for the conversion. Default is CP_UTF8.
   * \return The converted wide string.
   */
		static std::wstring stringToWstring(const std::string& str, int codepage = CP_UTF8);

		/**
   * \brief Convert a wide string to a string.
   * \param wstr The wide string to convert.
   * \param codepage The code page to use for the conversion. Default is CP_UTF8.
   * \return The converted string.
   */
		static std::string wstringToString(const std::wstring& wstr, int codepage = CP_UTF8);
};
