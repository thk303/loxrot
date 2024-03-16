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
#include <sstream>

/**
 * \class Logging
 * \brief A singleton class for logging.
 */
class Logging {
private:
    /**
     * \brief Private constructor for the singleton Logging class.
     */
    Logging();

    /**
     * \brief Destructor for the Logging class.
     */
    ~Logging();

    /**
     * \brief Log a message with a specific log level.
     * \param loglevel_ The log level.
     * \param message The message to log.
     */
    void _log(int loglevel_, std::wstring message);
    void _log(int loglevel_, std::string message);

    std::wofstream logstreamW; ///< Wide character log stream.
    std::ofstream logstream; ///< Log stream.
    static Logging* instance; ///< Singleton instance of the Logging class.
    static int loglevel; ///< Current log level.
    static std::wstring filename; ///< Log file name.
    static std::string syslogAddress; ///< Syslog server address.
    static int syslogPort; ///< Syslog server port.
    static std::string ownHostname; ///< Hostname of this machine.
    static int ownPid; ///< Process ID of this process.
    std::vector<std::wstring> loglevelsW = { L"DEBUG", L"INFO", L"WARNING", L"ERROR", L"FATAL" }; ///< Log levels in wide characters.
    std::vector<std::string> loglevels = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" }; ///< Log levels.
    void sendToSyslogViaUDP(const std::string& message); ///< Send a message to the syslog server via UDP.
    void sendToSyslogViaUDP(const std::wstring& message); ///< Send a wide character message to the syslog server via UDP.

public:
    /**
     * \struct LogLevel
     * \brief Struct for log level constants.
     */
    struct LogLevel {
        static const int none = -1;
        static const int debug = 0;
        static const int info = 1;
        static const int warning = 2;
        static const int error = 3;
        static const int fatal = 4;
    };

    /**
     * \brief Get the singleton instance of the Logging class.
     * \return The singleton instance of the Logging class.
     */
    static Logging* getInstance();

    /**
     * \brief Set the log options.
     * \param level The log level.
     * \param filename The log file name.
     */
    static void setLogOptions(int level, const std::wstring& filename);

    static void debug(std::wstring message); ///< Log a debug message.
    static void debug(std::string message); ///< Log a debug message.
    static void info(std::wstring message); ///< Log an info message.
    static void info(std::string message); ///< Log an info message.
    static void warning(std::wstring message); ///< Log a warning message.
    static void warning(std::string message); ///< Log a warning message.
    static void error(std::wstring message); ///< Log an error message.
    static void error(std::string message); ///< Log an error message.
    static void fatal(std::wstring message); ///< Log a fatal message.
    static void fatal(std::string message); ///< Log a fatal message.
};
