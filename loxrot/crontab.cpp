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

#include "crontab.h"
#include <sstream>
#include <iostream>
#include <windows.h>
#include "logging.h"
#include <regex>

Crontab::Crontab()
{
	last.tm_sec = 0;
	last.tm_min = 0;
	last.tm_hour = 0;
	last.tm_mday = 0;
	last.tm_mon = 0;
	last.tm_year = 0;
	last.tm_wday = 0;
	last.tm_yday = 0;
	last.tm_isdst = 0;
}

Crontab::~Crontab()
{
}

bool Crontab::isTimeToRotate()
{
	// Get the current time
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);

	// Check if the current time is in the crontab
    if (std::find(weekdays.begin(), weekdays.end(), ltm.tm_wday) != weekdays.end() &&
		std::find(months.begin(), months.end(), ltm.tm_mon + 1) != months.end() &&
		std::find(days.begin(), days.end(), ltm.tm_mday) != days.end() &&
		std::find(hours.begin(), hours.end(), ltm.tm_hour) != hours.end() &&
		std::find(minutes.begin(), minutes.end(), ltm.tm_min) != minutes.end())
    {
		// Check if the current time is different from the last time the roation was done
		Logging::debug(L"ltm.tm_min: " + std::to_wstring(ltm.tm_min) + L" last.tm_min: " + std::to_wstring(last.tm_min));
		if (ltm.tm_wday == last.tm_wday && ltm.tm_min == last.tm_min && ltm.tm_hour == last.tm_hour && ltm.tm_mday == last.tm_mday && ltm.tm_mon == last.tm_mon && ltm.tm_year == last.tm_year) {
			return false;
		}
		last.tm_year = ltm.tm_year;
		last.tm_mon = ltm.tm_mon;
		last.tm_mday = ltm.tm_mday;
		last.tm_hour = ltm.tm_hour;
		last.tm_min = ltm.tm_min;
		last.tm_wday = ltm.tm_wday;
		return true;
	}
    return false;
}

bool Crontab::parse(const std::wstring& crontabstring)
{
	// Clear the vectors
	minutes.clear();
	hours.clear();
	days.clear();
	months.clear();
	weekdays.clear();
	 
	// trim the string
	std::wstring::size_type start = crontabstring.find_first_not_of(L" ");
	std::wstring::size_type end = crontabstring.find_last_not_of(L" ");
	std::wstring trimmed = crontabstring.substr(start, end - start + 1);

	// split the string
	std::wstringstream ss(trimmed);
	std::vector<std::vector<int>*> fields = {&minutes, &hours, &days, &months, &weekdays};

	// parse the string
    int vecnum(0);
	std::wstring token;
	std::wsmatch match;
	while (getline(ss, token, L' ')) {
		if (token == L"*") {
			if (vecnum == 0) { // min
				for (int i = 0; i < 60; i++) {
					minutes.push_back(i);
				}
			}
			else if (vecnum == 1) { // hour
				for (int i = 0; i < 24; i++) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 2) { // day
				for (int i = 1; i < 32; i++) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 3) { // month 0-11
				for (int i = 0; i < 12; i++) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 4) { // weekday
				for (int i = 0; i < 7; i++) {
					fields[vecnum]->push_back(i);
				}
			}
		}
		else if (token.find(L',') != std::wstring::npos) {
			if (!std::regex_match(token, std::wregex(L"(\\d+,\\d+)(,\\d+)*"))) {
				return false;
			}
			std::wstringstream ss2(token);
			std::wstring token2;
			while (std::getline(ss2, token2, L',')) {
				if (token2.length() == 0) {
					return false;
				}
				if (vecnum == 0) { // min
					if (std::stoi(token2) < 0 || std::stoi(token2) > 59) {
						return false;
					}
				}
				else if (vecnum == 1) { // hour
					if (std::stoi(token2) < 0 || std::stoi(token2) > 23) {
						return false;
					}
				}
				else if (vecnum == 2) { // day
					if (std::stoi(token2) < 1 || std::stoi(token2) > 31) {
						return false;
					}
				}
				else if (vecnum == 3) { // month 0-11
					if (std::stoi(token2) < 0 || std::stoi(token2) > 11) {
						return false;
					}
				}
				else if (vecnum == 4) { // weekday
					if (std::stoi(token2) < 0 || std::stoi(token2) > 6) {
						return false;
					}
				}
				fields[vecnum]->push_back(std::stoi(token2));
			}
		}
		else if (token.find(L"-") != std::wstring::npos) {
			if (!std::regex_match(token, match, std::wregex(L"(\\d+)-(\\d+)"))) {
				return false;
			}
			int start(std::stoi(match[1]));
			end = std::stoi(match[2]);
			if (vecnum == 0) { // min
				if (start < 0 || start > 59 || end < 0 || end > 59) {
					return false;
				}
			}
			else if (vecnum == 1) { // hour
				if (start < 0 || start > 23 || end < 0 || end > 23) {
					return false;
				}
			}
			else if (vecnum == 2) { // day
				if (start < 1 || start > 31 || end < 1 || end > 31) {
					return false;
				}
			}
			else if (vecnum == 3) { // month 0-11
				if (start < 0 || start > 11 || end < 0 || end > 11) {
					return false;
				}
			}
			else if (vecnum == 4) { // weekday
				if (start < 0 || start > 6 || end < 0 || end > 6) {
					return false;
				}
			}
			for (int i = start; i <= end; i++) {
				fields[vecnum]->push_back(i);
			}
		}
		else if (token.find(L'/') != std::wstring::npos) {
			if (!std::regex_match(token, match, std::wregex(L"(\\d+|\\*+)\\/(\\d+)"))) {
				return false;
			}

			int start;
			if (match[1] == L"*") {
				if (vecnum == 2) {	// days start at 1
					start = 1;
				}
				else {				// rest starts at 0
					start = 0;
				}
			}
			else {
				start = std::stoi(match[1]);
			}

			int step = std::stoi(match[2]);
			if (step <= 0) {
				return false;
			}
			if (vecnum == 0) {	// minutes
				if (start < 0 || start > 59) {
					return false;
				}
				for (int i = start; i < 60; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 1) {	// hours
				if (start < 0 || start > 23) {
					return false;
				}
				for (int i = start; i < 24; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 2) {	// days
				if (start < 1 || start > 31) {
					return false;
				}
				for (int i = start; i < 32; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 3) {	// months
				if (start < 0 || start > 11) {
					return false;
				}
				for (int i = start; i < 12; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 4) {	// weekdays
				if (start < 0 || start > 6) {
					return false;
				}
				for (int i = start; i < 7; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
		}
		else if (token == L"") {
			continue;
		}
		else if(std::regex_match(token, std::wregex(L"\\d+"))) {
			if (vecnum == 0) {
				if (std::stoi(token) < 0 || std::stoi(token) > 59) {
					return false;
				}
			}
			else if (vecnum == 1) {
				if (std::stoi(token) < 0 || std::stoi(token) > 23) {
					return false;
				}
			}
			else if (vecnum == 2) {
				if (std::stoi(token) < 1 || std::stoi(token) > 31) {
					return false;
				}
			}
			else if (vecnum == 3) {
				if (std::stoi(token) < 0 || std::stoi(token) > 11) {
					return false;
				}
			}
			else if (vecnum == 4) {
				if (std::stoi(token) < 0 || std::stoi(token) > 6) {
					return false;
				}
			}
			fields[vecnum]->push_back(std::stoi(token));
		}
		else
			return false;
		vecnum++;
    }

	if (vecnum != 5) {
		return false;
	}

	// Check if all fields are set
	if (minutes.empty()) {
		return false;
	}
	if (hours.empty()) {
		return false;
	}
	if (days.empty()) {
		return false;
	}
	if (months.empty()) {
		return false;
	}
	if (weekdays.empty()) {
		return false;
	}
	return true;
}
