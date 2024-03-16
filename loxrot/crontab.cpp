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
#include "crontab.h"
#include <sstream>
#include <iostream>
#include <windows.h>
#include "logging.h"

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
	// Parse the crontab string
	std::wstringstream ss(crontabstring);
	std::vector<std::vector<int>*> fields = {&minutes, &hours, &days, &months, &weekdays};
    int vecnum(0);
	std::wstring token;
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
			else if (vecnum == 3) { // month
				for (int i = 1; i < 13; i++) {
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
			std::wstringstream ss2(token);
			std::wstring token2;
			while (std::getline(ss2, token2, L',')) {
				fields[vecnum]->push_back(std::stoi(token2));
			}
		}
		else if (token.find(L"-") != std::wstring::npos) {
			std::wstringstream ss2(token);
			std::wstring token2;
			while (std::getline(ss2, token2, L'-')) {
				int start = std::stoi(token2);
				std::getline(ss2, token2, L'-');
				int end = std::stoi(token2);
				for (int i = start; i <= end; i++) {
					fields[vecnum]->push_back(i);
				}
			}
		}
		else if (token.find(L'/') != std::wstring::npos) {
			std::wstringstream ss2(token);
			std::wstring token2;
			std::getline(ss2, token2, L'/');
			if (token2 == L"*") {
				token2 = L"0";
			}
			int start = std::stoi(token2);
			std::getline(ss2, token2, L'/');
			int step = std::stoi(token2);
			if (vecnum == 0) {
				for (int i = start; i < 60; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 1) {
				for (int i = start; i < 24; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 2) {
				for (int i = start; i < 32; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 3) {
				for (int i = start; i < 13; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
			else if (vecnum == 4) {
				for (int i = start; i < 7; i += step) {
					fields[vecnum]->push_back(i);
				}
			}
		}
		else {
			fields[vecnum]->push_back(std::stoi(token));
		}
		vecnum++;
    }
	// TODO: check if the crontab string is valid and return false if not or use exceptions
    return true;
}
