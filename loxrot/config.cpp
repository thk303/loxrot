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

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.
*/
#include "config.h"
#include <chrono>
#include <regex>
#include "logging.h"
#include "crontab.h"
#include <map>

Config::Config()
{
}

Config::~Config()
{
}

const std::map<std::wstring, std::wstring>& Config::getSection(const std::wstring& section)
{
    return configs[section].entries;
}

std::map<std::wstring, Config::Section>& Config::getConfigs()
{
    return configs;
}

// Converts a duration string to seconds
int Config::convertToSeconds(const std::wstring& duration) {
    std::wregex re(L"(\\d+)([mhdMy])");
    std::wsmatch match;

    if (std::regex_match(duration, match, re)) {
        int value = std::stoi(match[1].str());
        wchar_t unit = match[2].str()[0];

        switch (unit) {
        case L'm': // Minutes
            return std::chrono::minutes(value).count() * 60;
        case L'h': // Hours
            return std::chrono::hours(value).count() * 60 * 60;
        case L'd': // Days
            return std::chrono::duration<int, std::ratio<24 * 60 * 60>>(value).count();
        case L'M': // Months (30 days)
            return std::chrono::duration<int, std::ratio<30 * 24 * 60 * 60>>(value).count();
        case L'y': // Years (365 days)
            return std::chrono::duration<double, std::ratio<60 * 60 * 24 * 365>>(value).count();
        default:
            throw std::invalid_argument("Invalid unit in the age string in the config. Only 'm', 'h, 'd', 'M' or 'y' allowed.");
        }
    }
    else {
        throw std::invalid_argument("Invalid unit in the age string in the config. Only 'm', 'h, 'd', 'M' or 'y' allowed.");
    }
}

void Config::load(const std::wstring& configfile)
{
    Logging::debug(L"Entered parseConfig");
    std::wifstream file(configfile);
    std::wstring line;
    std::wstring section;
    while (std::getline(file, line)) {
        if (std::regex_match(line, std::wregex(L"(\\s*#.*)|(\\s*;.*)")))
            continue;
        std::wsmatch match;
        if (std::regex_match(line, match, std::wregex(L"^\\[(.+)\\]$"))) {
            section = match[1];
        }
        else {
            if (std::regex_match(line, match, std::wregex(L"^([A-Za-z]+)\\s*[\\=]{1}\\s*(.*)$"))) {
                std::wstring key = match[1];
                std::wstring value = match[2];
                if (key == L"KeepFiles") {
                    if (!regex_match(value, std::wregex(L"^(\\-*\\d+)$"))) {
                        std::wstring msg = L"Invalid value " + key + L" in section " + section + L" in config file " + configfile;
                        Logging::fatal(msg + L". Aborting program.");
                        throw std::runtime_error(std::string(msg.begin(), msg.end()));
                    }
                }
                else if (key == L"Simulation") {
                    if(value != L"true" && value != L"false") {
						std::wstring msg = L"Invalid value " + key + L" in section " + section + L" in config file " + configfile;
                        Logging::fatal(msg + L". Aborting program.");
                        throw std::runtime_error(std::string(msg.begin(), msg.end()));
					}
                }
                else if (key == L"MinAge") {
                    try {
						value = std::to_wstring(convertToSeconds(value));
					}
					catch (std::invalid_argument& e) {
						std::wstring msg = L"Invalid value of " + key + L" in section " + section + L" in config file " + configfile;
                        Logging::fatal(msg + L". Aborting program.");
                        throw std::runtime_error(std::string(msg.begin(), msg.end()));
                    }
                }
                else if(key == L"Timer") {
                    configs[section].crontab.parse(value);
				}
                configs[section].entries[key] = value;
            }
        }
    }
    // Check the configs and add default values if necessary
    for (std::map<std::wstring, Section>::iterator it = configs.begin(); it != configs.end(); it++) {
        if(it->second.entries.find(L"KeepFiles") == it->second.entries.end()) {
			it->second.entries[L"KeepFiles"] = L"-1";
		}
        if(it->second.entries.find(L"Directory") == it->second.entries.end()) {
			std::wstring msg = L"Directory not found in section " + it->first + L" in config file " + configfile;
            Logging::fatal(msg + L". Aborting program.");
			throw std::runtime_error(std::string(msg.begin(), msg.end()));
		}
        if(it->second.entries.find(L"FilePattern") == it->second.entries.end()) {
			std::wstring msg = L"FilePattern not found in section " + it->first + L" in config file " + configfile;
            Logging::fatal(msg + L". Aborting program.");
			throw std::runtime_error(std::string(msg.begin(), msg.end()));
		}
        if(it->second.entries.find(L"Timer") == it->second.entries.end()) {
            it->second.entries[L"Timer"] = L"0 * * * *";
        }
        if (it->second.entries.find(L"Simulation") == it->second.entries.end()) {
            it->second.entries[L"Simulation"] = L"false";
        }
        if (it->second.entries.find(L"MinAge") == it->second.entries.end()) {
            it->second.entries[L"MinAge"] = L"0m";
        }
	}
    Logging::debug(L"Leaving parseConfig");
}
