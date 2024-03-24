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
#ifdef WITH_ZLIB
#include <zlib.h>
#endif

// Constructor
Rotate::Rotate() {
}

// Destructor
Rotate::~Rotate() {
}

#ifdef WITH_ZLIB
bool Rotate::compressFile(const std::wstring& filename) {
	std::ifstream ifs(filename, std::ios::binary);
    gzFile gz = gzopen_w(std::wstring(filename + L".gz").c_str(), "wb");
    if (!gz) {
		Logging::error(L"Could not open " + filename + L".gz for writing");
        ifs.close();
        return false;
	}
    const int bufsize = 1024;
    char buffer[bufsize];
    memset(buffer, 0, bufsize);
    while (ifs.read(buffer, bufsize)) {
        gzwrite(gz, buffer, static_cast<int>(ifs.gcount()));
    }
	gzclose(gz);
	ifs.close();
    return true;
}
#endif

// Get a list of files in a directory that match a pattern
std::vector<std::wstring> Rotate::getFilesInDirectory(const std::wstring directory, const std::wstring pattern, bool returnFullPath) {
    std::vector<std::wstring> files;
    std::wregex re(pattern);
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && std::regex_match(entry.path().filename().wstring(), re)) {
            files.push_back(returnFullPath ? entry.path().wstring() : entry.path().filename().wstring());
        }
    }
    return files;
}

// Get the age of a file in seconds
long long Rotate::getFileAgeInSeconds(const std::wstring filename) {
	// Open the file
	HANDLE hFile = CreateFileW(filename.c_str(), FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		Logging::error(L"Could not open file " + filename + L" for getting creation time");

    FILETIME ftFile;
	// Get the creation time of the file
	if (!GetFileTime(hFile, &ftFile, NULL, NULL)) {
		CloseHandle(hFile);
		Logging::error(L"Could not get creation time of " + filename);
	}
	CloseHandle(hFile);

	// Get the current system time
	FILETIME ftNow;
	GetSystemTimeAsFileTime(&ftNow);

	// Convert FILETIME in ULARGE_INTEGER
	ULARGE_INTEGER creationULI, currentULI;
	creationULI.LowPart = ftFile.dwLowDateTime;
	creationULI.HighPart = ftFile.dwHighDateTime;
	currentULI.LowPart = ftNow.dwLowDateTime;
	currentULI.HighPart = ftNow.dwHighDateTime;

	// Calculate the difference in intervals of 100 nanoseconds
	ULONGLONG diff = currentULI.QuadPart - creationULI.QuadPart;

	// Convert the difference to seconds
	long long diffSeconds = diff / 10000000ULL;
    return diffSeconds;
}

// Set the creation time of a file
void Rotate::setCreationTime(const std::wstring& filename) {
    // Open the file
    HANDLE hFile = CreateFileW(filename.c_str(), FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        Logging::error(L"Could not open file " + filename + L" for setting creation time");

    // Get the current system time
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Set the creation time of the file
    if (!SetFileTime(hFile, &ft, NULL, NULL)) {
        CloseHandle(hFile);
        Logging::error(L"Could not set creation time of " + filename);
    }
    
    CloseHandle(hFile);
}

// Rotate a file based on a configuration
int Rotate::rotateFile(Config::Section& config) {
    // Initialize the total number of renames
    int renamesTotal = 0;
    try {
        // Get a list of files to process
        std::vector<std::wstring> files2process = getFilesInDirectory(config.entries[L"Directory"], config.entries[L"FilePattern"], true);
        // Process each file
        for (auto& file2process : files2process) {
            // Initialize the number of renames for this file
            int renames = 0;
            // If the file is too young to rotate, skip it
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
#if WITH_ZLIB
                else if (std::filesystem::exists(file2process + L"." + std::to_wstring(appendix) + L".gz")) {
                    files.push_back(file2process + L"." + std::to_wstring(appendix) + L".gz");
                }
#endif
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
            // If the file is old enough to rotate
            if (files.size() > 0) {
                int suffix(0);

                std::wstring new_file(L"");
                // For each file
                for (auto& file : files) {
                    std::wsmatch match;
                    if (std::regex_match(file, match, std::wregex(L"^(.+)\\.(\\d+)$"))) {
                        std::wstring base = match[1].str();
                        suffix = match[2].matched ? std::stoi(match[2].str()) + 1 : 0;

                        new_file = base + L"." + std::to_wstring(suffix);
                    }
#if WITH_ZLIB
                    else if (std::regex_match(file, match, std::wregex(L"^(.+)\\.(\\d+\\.gz)$"))) {
                        std::wstring base = match[1].str();
                        suffix = match[2].matched ? std::stoi(match[2].str()) + 1 : 0;

                        new_file = base + L"." + std::to_wstring(suffix) + L".gz";
                    }
#endif
                    else {
                        suffix = 0;
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
                    // If the file is the original file
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
                                // Set the creation time of the truncated file to now
                                setCreationTime(file);
                                Logging::info(L"Truncated " + file2process);
                            }
                            else {
                                Logging::info(L"Simulated copy of " + file + L" to " + new_file);
							}
                        }
                    }
                    // If the file is not the original file
                    else {
                        if (config.entries[L"Simulation"] != L"true") {
							std::filesystem::rename(file, new_file);
                            Logging::debug(L"Renamed " + file + L"to " + new_file);
#ifdef WITH_ZLIB
                            if ((suffix >= std::stoi(config.entries[L"FirstCompress"])) && (new_file.rfind(L".gz") != (new_file.length() - 3))) {
                                if (compressFile(new_file)) {
									Logging::info(L"Compressed " + new_file);
                                    std::filesystem::remove(new_file);
								}
                                else {
                                    Logging::error(L"Could not compress " + new_file);
                                }
							}
#endif
                        }
                        else {
                            Logging::info(L"Simulated rename of " + file + L" to " + new_file);
                        }
                    }
                    // Increment the number of renames
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

// Rotate files based on a configuration
void Rotate::doRotates(std::pair<std::wstring, Config::Section>* config) {
    // Log that we have entered the doRotates function
    Logging::debug(L"Entered doRotates");
    try {
        // If it is time to rotate
        if (config->second.crontab.isTimeToRotate()) {
            // Rotate the file
            rotateFile(config->second);
        }
    }
    // Catch any filesystem errors
    catch (std::filesystem::filesystem_error& e) {
        // Log the error
        Logging::error(L"Filesystem error: " + Tools::stringToWstring(e.what()));
    }
    // Catch any other exceptions
    catch (...) {
        // Log the error
        Logging::error(L"Unknown exception in doRotates");
    }
    // Log that we are leaving the doRotates function
    Logging::debug(L"Leaving doRotates");
}
