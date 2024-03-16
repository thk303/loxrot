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
#include "rotate.h"
#include "logging.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fstream>
#include <list>
#include <regex>
#include <windows.h>
#include "tools.h"

Rotate::Rotate() {
}

Rotate::~Rotate() {
}

std::vector<std::wstring> Rotate::getFilesInDirectory(const std::wstring directory, const std::wstring pattern, bool returnFullPath) {
    std::vector<std::wstring> files;
    std::wregex re(pattern);
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && std::regex_match(entry.path().filename().wstring(), re))
            files.push_back(returnFullPath ? entry.path().wstring() : entry.path().filename().wstring());
    }
    return files;
}

long long Rotate::getFileAgeInSeconds(const std::wstring filename) {
    std::filesystem::path file_path(filename);
    auto last_write_time = std::filesystem::last_write_time(file_path);
    auto now = std::chrono::system_clock::now();
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(last_write_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    auto duration = now - sctp;
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
}

int Rotate::rotateFile(Config::Section& config) {
    int renamesTotal = 0;
    try {
        std::vector<std::wstring> files2process = getFilesInDirectory(config.entries[L"Directory"], config.entries[L"FilePattern"], true);
        for (auto& file2process : files2process) {
            int renames = 0;
            if (getFileAgeInSeconds(file2process) < std::stoi(config.entries[L"MinAge"])) {
                if(config.entries[L"Simulation"] == L"true") {
					Logging::info(L"File " + file2process + L" is too young to rotate. Skipping.");
				}
                continue;
            }
            int appendix = 0;
            std::list<std::wstring> files;
            while (1) {
                std::wstring fff = file2process + L"." + std::to_wstring(appendix);
                if (std::filesystem::exists(file2process + L"." + std::to_wstring(appendix))) {
                    files.push_back(file2process + L"." + std::to_wstring(appendix));
                }
                else
                    break;
                appendix++;
            }
            files.sort(std::greater<std::wstring>());
            while (files.size() >= std::stoi(config.entries[L"KeepFiles"])) {
                if (files.size() == 0) {
                    break;
                }
                else {
                    if (config.entries[L"Simulation"] != L"true") {
                        std::filesystem::remove(files.front());
                    }
                    else {
                        Logging::info(L"Simulated removal of " + files.front());
                    }
                    files.pop_front();
                }
            }
            files.push_back(file2process);
            if (files.size() > 0) {
                std::wstring pattern = L"^(.+)\\.(\\d+)$";
                std::wregex re(pattern);

                std::wstring new_file(L"");
                for (auto& file : files) {
                    std::wsmatch match;
                    auto matched = std::regex_match(file, match, re);
                    if (matched) {
                        std::wstring base = match[1].str();
                        int number = match[2].matched ? std::stoi(match[2].str()) + 1 : 0;

                        new_file = base + L"." + std::to_wstring(number);
                    }
                    else {
                        new_file = file + L".0";
                    }

                    // Copy original file to new file and truncate original file to keep permissions
                    if (std::stoi(config.entries[L"KeepFiles"]) == -1) {
                        if (config.entries[L"Simulation"] != L"true") {
                            std::filesystem::remove(file2process);
                        }
                        else {
                            Logging::info(L"Simulated removal of " + file2process);
                        }
                        return 1;
                    }
                    if (file == file2process) {
                        if (std::stoi(config.entries[L"KeepFiles"]) == -1) {
                            if (config.entries[L"Simulation"] != L"true") {
                                std::filesystem::remove(file2process);
                            }
							else {
								Logging::info(L"Simulated removal of " + file2process);
							}
                            Logging::debug(L"Deleted " + file2process);
                        }
                        else {
                            if (config.entries[L"Simulation"] != L"true") {
                                if (std::stoi(config.entries[L"KeepFiles"]) > 0) {
                                    std::filesystem::copy(file, new_file);
                                }
                                std::ofstream ofs(file, std::ios::trunc);
                                ofs.close();
                                Logging::info(L"Truncated " + file2process);
                            }
                            else {
                                Logging::info(L"Simulated copy of " + file + L" to " + new_file);
							}
                        }
                    }
                    else {
                        if (config.entries[L"Simulation"] != L"true") {
							std::filesystem::rename(file, new_file);
                            Logging::debug(L"Renamed " + file + L"to " + new_file);
                        }
                        else {
                            Logging::info(L"Simulated rename of " + file + L" to " + new_file);
                        }
                    }
                    renames++;
                }
            }
            renamesTotal += renames;
            if (renames > 0) {
                if (config.entries[L"Simulation"] != L"true") {
                    Logging::info(L"Rotated " + file2process);
                }
                else {
                    Logging::info(L"Simulated rotation of " + file2process + L" done.");
                }
            }
        }
    }
    catch (const std::regex_error& e) {
        std::cout << "regex_error caught: " << e.what() << '\n';
    }
    return renamesTotal;
}

void Rotate::doRotates(std::pair<std::wstring, Config::Section>* config) {
    Logging::debug(L"Entered doRotates");
    try {
        if (config->second.crontab.isTimeToRotate()) {
            rotateFile(config->second);
        }
    }
    catch (std::filesystem::filesystem_error& e) {
        Logging::error(L"Filesystem error: " + Tools::stringToWstring(e.what()));
    }
    catch (...) {
        Logging::error(L"Unknown exception in doRotates");
    }
    Logging::debug(L"Leaving doRotates");
}
