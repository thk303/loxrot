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
#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib
#include "logging.h"
#include "version.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "tools.h"

Logging* Logging::instance = nullptr;
int Logging::loglevel = LogLevel::info;
std::wstring Logging::filename = L"";
std::string Logging::syslogAddress = "127.0.0.1";
int Logging::syslogPort = 514;
std::string Logging::ownHostname = "";
int Logging::ownPid = 0;

Logging::Logging() {
    if(filename == L"stdout" || filename.empty()) {
		logstreamW = std::wofstream(L"CON");
		logstream = std::ofstream("CON");
		return;
	}
    if (filename.starts_with(L"syslog://")) {
        syslogAddress = Tools::wstringToString(filename.substr(9));
        if(syslogAddress.find(":") != std::string::npos) {
			syslogPort = std::stoi(syslogAddress.substr(syslogAddress.rfind(":") + 1));
			syslogAddress = syslogAddress.substr(0, syslogAddress.rfind(":"));
		}
		else {
			syslogPort = 514;
		}
        gethostname(ownHostname.data(), 256);
        ownHostname.resize(strlen(ownHostname.c_str()));
        ownPid = GetCurrentProcessId();
    }
    else {
        logstreamW = std::wofstream(filename, std::ios::app);
        logstream = std::ofstream(filename, std::ios::app);
    }
}

Logging::~Logging() {
    logstream.close();
}

void Logging::_log(int loglevel_, std::wstring message) {
    if (loglevel_ < loglevel) {
        return;
    }
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    std::wstringstream date, time;
    date << ltm.tm_year + 1900 << L"-" << std::setw(2) << std::setfill(L'0') << ltm.tm_mon + 1 << L"-" << std::setw(2) << std::setfill(L'0') << ltm.tm_mday;
    time << std::setw(2) << std::setfill(L'0') << ltm.tm_hour << L":" << std::setw(2) << std::setfill(L'0') << ltm.tm_min << L":" << std::setw(2) << std::setfill(L'0') << ltm.tm_sec;
    if(filename.starts_with(L"syslog://")) {
        std::wstring msg = loglevelsW[loglevel_] + L" " + message;
        std::wstring ownHostnameW = Tools::stringToWstring(ownHostname);
        msg.insert(0, ownHostnameW + PROGRAMNAMEW + L"[" + std::to_wstring(ownPid) + L"]: ");
        sendToSyslogViaUDP(msg);
	}
    else {
        logstreamW << date.str() << L" " << time.str() << L" " << loglevelsW[loglevel_] << L" " << message << std::endl;
    }
}

void Logging::_log(int loglevel_, std::string message) {
    if (loglevel_ < loglevel) {
        return;
    }
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    std::stringstream date, time;
    date << ltm.tm_year + 1900 << "-" << std::setw(2) << std::setfill('0') << ltm.tm_mon + 1 << "-" << std::setw(2) << std::setfill('0') << ltm.tm_mday;
    time << std::setw(2) << std::setfill('0') << ltm.tm_hour << ":" << std::setw(2) << std::setfill('0') << ltm.tm_min << ":" << std::setw(2) << std::setfill('0') << ltm.tm_sec;
    if (filename.starts_with(L"syslog://")) {
        std::string msg = loglevels[loglevel_] + " " + message;
        msg.insert(0, ownHostname + PROGRAMNAME + "[" + std::to_string(ownPid) + "]: ");
        sendToSyslogViaUDP(msg);
    }
    else {
        logstream << date.str() << " " << time.str() << " " << loglevels[loglevel_] << " " << message << std::endl;
    }
}

Logging* Logging::getInstance() {
    if (Logging::instance == nullptr) {
        Logging::instance = new Logging();
    }
    return Logging::instance;
}

void Logging::setLogOptions(int level, const std::wstring& filename) {
    loglevel = level;
    Logging::filename = filename;
}

void Logging::sendToSyslogViaUDP(const std::string& message) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(syslogPort);
    if (inet_pton(AF_INET, syslogAddress.c_str(), &(servaddr.sin_addr)) <= 0) {
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    if (sendto(sockfd, message.c_str(), message.size(), 0,
        (SOCKADDR*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
    }

    closesocket(sockfd);
    WSACleanup();
}

void Logging::sendToSyslogViaUDP(const std::wstring& message) {
	sendToSyslogViaUDP(Tools::wstringToString(message));
}

void Logging::debug(std::wstring message) {
    Logging::getInstance()->_log(LogLevel::debug, message);
}

void Logging::debug(std::string message) {
    Logging::getInstance()->_log(LogLevel::debug, message);
}

void Logging::info(std::wstring message) {
    Logging::getInstance()->_log(LogLevel::info, message);
}

void Logging::info(std::string message) {
    Logging::getInstance()->_log(LogLevel::info, message);
}

void Logging::warning(std::wstring message) {
    Logging::getInstance()->_log(LogLevel::warning, message);
}

void Logging::warning(std::string message) {
    Logging::getInstance()->_log(LogLevel::warning, message);
}

void Logging::error(std::wstring message) {
    Logging::getInstance()->_log(LogLevel::error, message);
}

void Logging::error(std::string message) {
    Logging::getInstance()->_log(LogLevel::error, message);
}

void Logging::fatal(std::wstring message) {
    Logging::getInstance()->_log(LogLevel::fatal, message);
}

void Logging::fatal(std::string message) {
    Logging::getInstance()->_log(LogLevel::fatal, message);
}

