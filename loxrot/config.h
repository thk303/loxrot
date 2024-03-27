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
#include "crontab.h"
#include <string>
#include <map>

/**
 * \class Config
 * \brief A class to handle configuration.
 */
class Config
{
public:
    /**
     * \brief Default constructor for Config.
     */
    Config();

    /**
     * \brief Destructor for Config.
     */
    ~Config();

    /**
     * \class Section
     * \brief A class to handle a section of the configuration.
     */
    class Section {
    public:
        /**
         * \brief Default constructor for Section.
         */
        Section() {};

        /**
         * \brief Destructor for Section.
         */
        ~Section() {};

        std::map<std::wstring, std::wstring> entries; ///< Map of entries in the section.
        Crontab crontab; ///< Crontab for the section.
    };

    /**
     * \brief Load configuration from a file.
     * \param configfile The path to the configuration file.
     */
    void load(const std::wstring& configfile);

    /**
     * \brief Get a section from the configuration.
     * \param section The name of the section.
     * \return A map of entries in the section.
     */
    const std::map<std::wstring, std::wstring>& getSection(const std::wstring& section);

    /**
     * \brief Get all configurations.
     * \return A map of all configurations.
     */
    std::map<std::wstring, Config::Section>& getConfigs();

#ifndef UNITTEST
private:
#endif
    /**
     * \brief Convert a duration string to seconds.
     * \param duration The duration string.
     * \return The duration in seconds.
     */
    int convertToSeconds(const std::wstring& duration);

    std::map<std::wstring, Section> configs; ///< Map of all configurations.
    std::wstring configfile; ///< The path to the configuration file.
};

