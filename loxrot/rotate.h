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
#include <chrono>
#include <string>
#include <map>
#include "config.h"

// The rotation functionality
/**
 * \class Rotate
 * \brief A class to handle file rotation.
 */
class Rotate
{
public:
    /**
     * \brief Default constructor for Rotate.
     */
    Rotate();

    /**
     * \brief Destructor for Rotate.
     */
    ~Rotate();

    /**
     * \brief Perform file rotations based on a configuration.
     * \param config The configuration to use for rotations.
     */
    void doRotates(std::pair<std::wstring, Config::Section>* config);

private:
    /**
     * \brief Get files in a directory that match a pattern.
     * \param directory The directory to search.
     * \param pattern The pattern to match.
     * \param returnFullPath Whether to return the full path of the files.
     * \return A vector of matching file names.
     */
    std::vector<std::wstring> getFilesInDirectory(const std::wstring directory, const std::wstring pattern, bool returnFullPath = false);

    /**
     * \brief Get the age of a file in seconds.
     * \param filename The name of the file.
     * \return The age of the file in seconds.
     */
    long long getFileAgeInSeconds(const std::wstring filename);

    /**
     * \brief Set the creation time of the truncated file to now.
     * \param filename The name of the file.
     * \return void
     */
    void setCreationTime(const std::wstring& filename);

    /**
     * \brief Rotate a file based on a configuration.
     * \param config The configuration to use for rotation.
     * \return The status of the rotation.
     */
    int rotateFile(Config::Section& config);
};

