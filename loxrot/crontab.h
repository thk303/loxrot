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
#pragma once
#include <string>
#include <vector>

/**
 * \class Crontab
 * \brief A class to handle crontab scheduling.
 */
class Crontab
{
public:
    /**
     * \brief Default constructor for Crontab.
     */
    Crontab();

    /**
     * \brief Destructor for Crontab.
     */
    ~Crontab();

    /**
     * \brief Parse a crontab string.
     * \param crontabstring The crontab string to parse.
     * \return True if the parsing was successful, false otherwise.
     */
    bool parse(const std::wstring& crontabstring);

    /**
     * \brief Check if it's time to rotate.
     * \return True if it's time to rotate, false otherwise.
     */
    bool isTimeToRotate();
#ifndef UNITTEST
private:
#endif
    tm last; ///< The last time the crontab was checked.
    std::wstring crontabstring; ///< The crontab string.
    std::vector<int> minutes; ///< The minutes field of the crontab.
    std::vector<int> hours; ///< The hours field of the crontab.
    std::vector<int> days; ///< The days field of the crontab.
    std::vector<int> months; ///< The months field of the crontab.
    std::vector<int> weekdays; ///< The weekdays field of the crontab.
};

