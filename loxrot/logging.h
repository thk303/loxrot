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
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
//#include <windows.h>
#include <sstream>

// The logging class is a singleton.
class Logging {
private:
    Logging();
    ~Logging();

    void _log(int loglevel_, std::wstring message);
    void _log(int loglevel_, std::string message);

    std::wofstream logstreamW;
    std::ofstream logstream;
    static Logging* instance;
    static int loglevel;
    static std::wstring filename;
    static std::string syslogAddress;
    static int syslogPort;
    static std::string ownHostname;
    static int ownPid;
    std::vector<std::wstring> loglevelsW = { L"DEBUG", L"INFO", L"WARNING", L"ERROR", L"FATAL" };
    std::vector<std::string> loglevels = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    void sendToSyslogViaUDP(const std::string& message);
    void sendToSyslogViaUDP(const std::wstring& message);

public:
    struct LogLevel {
        static const int none = -1;
        static const int debug = 0;
        static const int info = 1;
        static const int warning = 2;
        static const int error = 3;
        static const int fatal = 4;
    };

    static Logging* getInstance();
    static void setLogOptions(int level, const std::wstring& filename);
    static void debug(std::wstring message);
    static void debug(std::string message);
    static void info(std::wstring message);
    static void info(std::string message);
    static void warning(std::wstring message);
    static void warning(std::string message);
    static void error(std::wstring message);
    static void error(std::string message);
    static void fatal(std::wstring message);
    static void fatal(std::string message);
};
