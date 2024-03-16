#include "timerparser.h"
#include <sstream>
#include <iostream>

namespace timerparser {
    namespace internal {
        string wstring2string(const wstring& wstr) {
            // Convert a Unicode string to an ASCII string using widechartomultibyte
            DWORD len(0);
            int neededLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), nullptr, 0, NULL, NULL);
            vector<char> buffer(neededLen + 1);
            memset(buffer.data(), 0, buffer.size());
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), buffer.data(), neededLen, NULL, NULL);
            return string(buffer.begin(), buffer.end());
        }

        void days2numbers(const vector<wstring> singledays, vector<int>& days) {
            vector<wstring> weekdays = { L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
            wsmatch match;
            if (singledays.size() == 1 && singledays[0] == L"*") {
                days.clear();
                return;
            }
            auto it = find(singledays.begin(), singledays.end(), L"...");
            if (it != singledays.end()) {
                auto itstartday = find(weekdays.begin(), weekdays.end(), singledays[0]);
                auto itendday = find(weekdays.begin(), weekdays.end(), singledays[2]);
                if (itstartday >= itendday) {
					throw timestring_error("Invalid daypart: " + internal::wstring2string(singledays[0]) + "..." + internal::wstring2string(singledays[2]));
				}
                for (auto it = itstartday; it <= itendday; it++) {
					days.push_back(distance(weekdays.begin(), it));
				}
                return;
            }
            for (const wstring& day : singledays) {
                auto it = find(weekdays.begin(), weekdays.end(), day);
                if (it != weekdays.end()) {
                    days.push_back(distance(weekdays.begin(), it));
                }
                else {
                    throw timestring_error("Invalid daypart: " + internal::wstring2string(day));
                }
            }
        }

        void parseDays(const wstring daypart, vector<int>& days) {
            vector<wstring> weekdays = { L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
            wsmatch match;
            if (regex_match(daypart, match, wregex(L"^(\\*|Mon|Tue|Wed|Thu|Fri|Sat|Sun)+$"))) {
                days2numbers(vector<wstring> {daypart}, days);
			}
            else if (regex_match(daypart, match, wregex(L".*,.*"))) {
                vector<wstring>singledays;
                wstring token(L"");
                wstringstream ss(daypart);
                while (getline(ss, token, L',')) {
					singledays.push_back(token);
				}
				days2numbers(singledays, days);
            }
            else if (regex_match(daypart, match, wregex(L"(Mon|Tue|Wed|Thu|Fri|Sat|Sun)(\\.\\.\\.)(Mon|Tue|Wed|Thu|Fri|Sat|Sun)"))) {
				days2numbers(vector<wstring> {match[1], match[2], match[3]}, days);
			}
            else if (daypart != L"*") {
				throw timestring_error("Invalid daypart: " + internal::wstring2string(daypart));
			}
		}

        void parseDate(const wstring datepart, vector<int>& years, vector<int>& months, vector<int>& days) {
            // Aktuell noch keine Unterstützung für Teiler und Schrittweiten
			wsmatch match;
            if (regex_match(datepart, match, wregex(L"^(\\*|\\d{4})-(\\*|\\d{1,2})-(\\*|\\d{1,2})$"))) {
                if (match[1] != L"*") {
					years.push_back(stoi(match[1].str()));
				}
                if (match[2] != L"*") {
					months.push_back(stoi(match[2].str()));
				}
                if (match[3] != L"*") {
					days.push_back(stoi(match[3].str()));
				}
			}
		}
        
        void parseTime(const wstring datepart, vector<int>& hours, vector<int>& minutes, vector<int>& seconds) {
            // Aktuell noch keine Unterstützung für Teiler und Schrittweiten
			wsmatch match;
            if (regex_match(datepart, match, wregex(L"^(\\*|\\d{1,2}):(\\*|\\d{1,2}):(\\*|\\d{1,2})$"))) {
                if (match[1] != L"*") {
					hours.push_back(stoi(match[1].str()));
				}
                if (match[2] != L"*") {
					minutes.push_back(stoi(match[2].str()));
				}
                if (match[3] != L"*") {
					seconds.push_back(stoi(match[3].str()));
				}
			}
		}
    }

    bool isTimeToRotate(const wstring& rotateTimes) {
        // rotateTimes is a systemd-style timer OnCalendar-definition
        vector<int> weekdays;
        vector<int> years;
        vector<int> months;
        vector<int> days;
        vector<int> hours;
        vector<int> minutes;
        vector<int> seconds;

        wstring daypart(L"*");
        wstring datepart(L"*-*-*");
        wstring timepart(L"*:*:*");

        wsmatch match;
        if (regex_match(rotateTimes, match, wregex(L"(.*)\\s(.*)\\s+(.*)"))) {
            daypart = match[1].str();
            datepart = match[2].str();
            timepart = match[3].str();
        }
        else if (regex_match(rotateTimes, match, wregex(L"(.*)\\s+(.*)"))) {
            datepart = match[1].str();
            timepart = match[2].str();
        }
        else {
            throw timestring_error("Invalid timestring: " + internal::wstring2string(rotateTimes));
        }

        internal::parseDays(daypart, weekdays);
        internal::parseDate(datepart, years, months, days);
        internal::parseTime(timepart, hours, minutes, seconds);

        // Get current date and time
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);
        if (weekdays.size() == 0 || find(weekdays.begin(), weekdays.end(), ltm.tm_wday) != weekdays.end()) {
            if (years.size() == 0 || find(years.begin(), years.end(), ltm.tm_year + 1900) != years.end()) {
                if (months.size() == 0 || find(months.begin(), months.end(), ltm.tm_mon + 1) != months.end()) {
                    if (days.size() == 0 || find(days.begin(), days.end(), ltm.tm_mday) != days.end()) {
                        if (hours.size() == 0 || find(hours.begin(), hours.end(), ltm.tm_hour) != hours.end()) {
                            if (minutes.size() == 0 || find(minutes.begin(), minutes.end(), ltm.tm_min) != minutes.end()) {
                                if (seconds.size() == 0 || find(seconds.begin(), seconds.end(), ltm.tm_sec) != seconds.end()) {
									return true;
								}
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
}