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

#include "logging.h"
#include "config.h"
#include "rotate.h"
#include "version.h"
#include <iostream>
#include <windows.h>
#include <thread>
#include <filesystem>

// Define the service status and service status handle
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

// Forward declaration of ServiceMain and ControlHandler functions
void  ServiceMain(int argc, wchar_t** argv);
void  ControlHandler(DWORD request);

// Define a struct to hold command line arguments
struct Args {
    std::wstring configfile = L""; // Configuration file path
    std::wstring logfile = PROGRAMNAMEW + L".log"; // Log file path
    int loglevel = Logging::LogLevel::none; // Log level
    std::wstring loglevelname = L"none"; // Log level name, used for installing the service
    bool service = false; // Flag to indicate if the program should run as a service
    bool foreground = false; // Flag to indicate if the program should run in the foreground
    bool installservice = false; // Flag to indicate if the service should be installed
    bool uninstallservice = false; // Flag to indicate if the service should be uninstalled
};

// Function to parse command line arguments
bool parseArgs(int argc, wchar_t** argv, Args* args) {
    // Create a string stream for the help text
    std::wstringstream helptext;
    // Set the default log level to info
    args->loglevel = Logging::LogLevel::info;
    // Populate the help text with usage instructions
    helptext << PROGRAMNAMEW << L" v" << VERSION << std::endl
        << L"Usage: " + PROGRAMNAMEW + L" --config <configfile> [--foreground] [--logfile <logfile|:stdout|syslog://<ip>:[<port>]] [--loglevel <loglevel>] [--installservice|--uninstallservice]" << std::endl;
    // If there are less than 2 command line arguments, print the help text
    if (argc < 2) {
        std::wcout << helptext.str() << std::endl;
    }
    // Loop through the command line arguments
    for (int i = 1; i < argc; i++) {
        // If the argument is "--config"
        if (wcscmp(argv[i], L"--config") == 0) {
            // If there is another argument after this one
            if (i + 1 < argc) {
                // Set the config file path to the next argument
                args->configfile = argv[i + 1];
                i++;
            }
            else {
                // If there is no argument after this one, print an error message and return false
                std::wcout << L"Missing argument for --config" << std::endl;
                return false;
            }
        }
        // If the argument is "--logfile"
        else if (wcscmp(argv[i], L"--logfile") == 0) {
            // If there is another argument after this one
            if (i + 1 < argc) {
                // Set the log file path to the next argument
                args->logfile = argv[i + 1];
                i++;
            }
            else {
                // If there is no argument after this one, print an error message and return false
                std::wcout << L"Missing argument for --logfile" << std::endl;
                return false;
            }
        }
        // If the argument is "--loglevel"
        else if (wcscmp(argv[i], L"--loglevel") == 0) {
            // If there is another argument after this one
            if (i + 1 < argc) {
                // Set the log level name to the next argument
                args->loglevelname = argv[i + 1];
                // Set the log level based on the next argument
                if (!wcscmp(argv[i + 1], L"none")) {
                    args->loglevel = Logging::LogLevel::none;
                }
                else if (!wcscmp(argv[i + 1], L"debug")) {
                    args->loglevel = Logging::LogLevel::debug;
                }
                else if (!wcscmp(argv[i + 1], L"info")) {
                    args->loglevel = Logging::LogLevel::info;
                }
                else if (!wcscmp(argv[i + 1], L"warning")) {
                    args->loglevel = Logging::LogLevel::warning;
                }
                else if (!wcscmp(argv[i + 1], L"error")) {
                    args->loglevel = Logging::LogLevel::error;
                }
                else if (!wcscmp(argv[i + 1], L"fatal")) {
                    args->loglevel = Logging::LogLevel::fatal;
                }
                else {
                    // If the next argument is not a valid log level, print an error message and return false
                    std::wcout << L"Wrong argument for --loglevel. Valid values are 'none', 'debug', 'info', 'warning', 'error' and 'fatal'." << std::endl;
                    return false;
                }
            }
            else {
                // If there is no argument after this one, print an error message and return false
                std::wcout << L"Missing argument for --loglevel. Valid values are 'none', 'debug', 'info', 'warning', 'error' and 'fatal'." << std::endl;
                return false;
            }
            i++;
        }
        // If the argument is "--service", set the service flag to true
        else if (wcscmp(argv[i], L"--service") == 0) {
            args->service = true;
        }
        // If the argument is "--foreground", set the foreground flag to true
        else if (wcscmp(argv[i], L"--foreground") == 0) {
            args->foreground = true;
        }
        // If the argument is "--installservice", set the install service flag to true
        else if (wcscmp(argv[i], L"--installservice") == 0) {
            args->installservice = true;
        }
        // If the argument is "--uninstallservice", set the uninstall service flag to true
        else if (wcscmp(argv[i], L"--uninstallservice") == 0) {
            args->uninstallservice = true;
        }
        else {
            // If the argument is not recognized, print the help text and an error message, and return false
            std::wcout << helptext.str() << std::endl << L"Unknown argument " << argv[i] << std::endl;
            return false;
        }
    }
    // Check for the existance of neccessary arguments
    if (args->configfile == L"" && args->uninstallservice == false) {
        std::wcout << L"Missing argument --config" << std::endl;
        return false;
    }

    // If all arguments were parsed successfully, return true
    return true;
}

// The main function for the service
void ServiceMain(int argc, wchar_t** argv)
{
    // Get the command line as a wide string
    std::wstring cmdline = GetCommandLine();
    // Initialize variables to hold the number of arguments and the argument vector
    int argc_ = 0;
    wchar_t** argv_ = CommandLineToArgvW(cmdline.c_str(), &argc_);
    // If CommandLineToArgvW failed, log an error and return
    if (argv_ == NULL) {
		Logging::error(L"CommandLineToArgvW failed");
		return;
	}
    // Initialize an Args struct to hold the command line arguments
    Args args;
    // Parse the command line arguments
    if (parseArgs(argc_, argv_, &args)) {
        // Set the log options based on the parsed arguments
        Logging::setLogOptions(args.loglevel, args.logfile);

        // Initialize a Config object to hold the configuration
        Config config;
        // Try to load the configuration from the config file specified in the arguments
        try {
            config.load(args.configfile);
        }
        // If loading the configuration fails, log a fatal error, throw a runtime error and return
        catch(std::invalid_argument& e) {
			Logging::fatal(e.what());
            throw std::runtime_error(e.what());
		}
        // Set the service status to pending
        ServiceStatus.dwServiceType = SERVICE_WIN32;
        ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwServiceSpecificExitCode = 0;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;

        // Register the service control handler
        hStatus = RegisterServiceCtrlHandlerW(PROGRAMNAMEW.c_str(), (LPHANDLER_FUNCTION)ControlHandler);

        // Set the service status to running
        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(hStatus, &ServiceStatus);

        // Initialize a Rotate object to handle log rotation
        Rotate rotate;
        // While the service is running
        while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            // For each section in the configuration
            for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                // Perform log rotation
                rotate.doRotates((std::pair<std::wstring, Config::Section>*)(&(*it)));
            }
            // If there are no sections in the configuration, log an error and return
            if (config.getConfigs().size() == 0) {
                Logging::error(L"No sections found in config file");
                return;
            }
            // Log that the service is still running
            Logging::debug(L"Service still running...");
            // Sleep for 1 second
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // Log that the service is leaving the ServiceMain function
        Logging::debug(L"Leaving ServiceMain");
    }
}

// Function to handle service control requests
void ControlHandler(DWORD request)
{
    // Handle the request based on its type
    switch (request)
    {
    // If the request is to stop the service
    case SERVICE_CONTROL_STOP:
        // Set the exit code to 0 and the current state to stopped
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        // Update the service status
        SetServiceStatus(hStatus, &ServiceStatus);
        // Log that the service has stopped
        Logging::info(L"Service stopped");
        return;

    // If the request is to shut down the service
    case SERVICE_CONTROL_SHUTDOWN:
        // Set the exit code to 0 and the current state to stopped
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        // Update the service status
        SetServiceStatus(hStatus, &ServiceStatus);
        // Log that the service has been shut down
        Logging::info(L"Service shut down");
        return;

    default:
        break;
    }

    // Update the service status
    SetServiceStatus(hStatus, &ServiceStatus);
}

// The main function for the program
int wmain(int argc, wchar_t** argv) {
    // Initialize an Args struct to hold the command line arguments
    Args args;
    // Parse the command line arguments
    if(parseArgs(argc, argv, &args)) {
        // Set the log options based on the parsed arguments
        Logging::setLogOptions(args.loglevel, args.logfile);
        // Try to start the program
        try {
            // Log that the program has started
            Logging::info(PROGRAMNAMEW + L" " + VERSION + L" started");
            // If the install service flag is set
            if (args.installservice) {
                // Log that the service is being installed
                std::wcout << L"Installing service" << std::endl;
                // Open the service control manager
                SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
                // If the service control manager was opened successfully
                if (schSCManager) {
                    // Create the command line for the service
                    std::wstring path = L"\"" + std::filesystem::absolute(argv[0]).wstring() + L"\" --service --config " + args.configfile + L" --logfile " + args.logfile + L" --loglevel " + args.loglevelname;
                    // Create the service
                    SC_HANDLE schService = CreateService(schSCManager, PROGRAMNAMEW.c_str(), PROGRAMNAMEW.c_str(), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path.c_str(), NULL, NULL, NULL, NULL, NULL);
                    // If the service was created successfully
                    if (schService) {
                        // Log that the service was installed
                        Logging::info(L"Service installed");
                        // Close the service handle
                        CloseServiceHandle(schService);
                    }
                    else {
                        // If the service was not created successfully, log an error
                        std::wcout << L"CreateService failed" << std::endl;
                    }
                    // Close the service control manager handle
                    CloseServiceHandle(schSCManager);
                }
                else {
                    // If the service control manager was not opened successfully, log an error
                    std::wcout << L"OpenSCManager failed" << std::endl;
                }
            }
            // If the uninstall service flag is set
            else if (args.uninstallservice) {
                // Log that the service is being uninstalled
                std::wcout << L"Uninstalling service" << std::endl;
                // Open the service control manager
                SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
                // If the service control manager was opened successfully
                if (schSCManager) {
                    // Open the service
                    SC_HANDLE schService = OpenService(schSCManager, PROGRAMNAMEW.c_str(), SERVICE_ALL_ACCESS);
                    // If the service was opened successfully
                    if (schService) {
                        // Delete the service
                        if (DeleteService(schService)) {
                            // If the service was deleted successfully, log that the service was uninstalled
                            std::wcout << L"Service uninstalled" << std::endl;
                        }
                        else {
                            // If the service was not deleted successfully, log an error
                            std::wcout << L"DeleteService failed" << std::endl;
                        }
                        // Close the service handle
                        CloseServiceHandle(schService);
                    }
                    else {
                        // If the service was not opened successfully, log an error
                        std::wcout << L"OpenService failed" << std::endl;
                    }
                }
            }
            // If neither the install service flag nor the uninstall service flag is set
            else {
                // If the service flag is set
                if (args.service) {
                    // Log that the program is starting as a service
                    Logging::info(L"Starting as service");
                    // Initialize the service table
                    SERVICE_TABLE_ENTRY ServiceTable[2];
                    ServiceTable[0].lpServiceName = (LPWSTR)PROGRAMNAME.c_str();
                    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONW)ServiceMain;

                    ServiceTable[1].lpServiceName = NULL;
                    ServiceTable[1].lpServiceProc = NULL;

                    // Start the service control dispatcher
                    StartServiceCtrlDispatcher(ServiceTable);
                    // Log that the service has started
                    Logging::debug(L"Service after StartServiceCtrlDispatcher");
                }
                else {
                    // If the service flag is not set, log that the program is starting as a console application
                    Logging::info(L"Starting as console application");
                    // Initialize a Config object to hold the configuration
                    Config config;
                    // Try to load the configuration from the config file specified in the arguments
                    try {
                        config.load(args.configfile);
                    }
                    // If loading the configuration fails, log a fatal error, throw a runtime error and return
                    catch (std::invalid_argument& e) {
                        Logging::fatal(e.what());
                        throw std::runtime_error(e.what());
                    }
                    // If there are no sections in the configuration, log a fatal error, throw a runtime error and return
                    if (config.getConfigs().size() == 0) {
                        Logging::fatal(L"No sections found in config file");
                        throw std::runtime_error("No sections found in config file");
                    }
                    // For each section in the configuration
                    for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                        // Log that the section is being checked
                        Logging::info(L"Checking " + it->first);
                    }
                    // Initialize a Rotate object to handle log rotation
                    Rotate rotate;
                    // While the program is running
                    while (1) {
                        // For each section in the configuration
                        for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                            // Perform log rotation
                            rotate.doRotates((std::pair<std::wstring, Config::Section>*)(&(*it)));
                        }
                        // If the foreground flag is set, sleep for 1 second
                        if (args.foreground) {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                        else {
                            // If the foreground flag is not set, break the loop
                            break;
                        }

                    }
                    // Log that the program has finished
                    Logging::info(PROGRAMNAMEW + L" " + VERSION + L" finished");
                }
            }
            // If the program ran successfully, return 0
            return 0;
        }
        // If an exception was thrown
        catch (std::runtime_error& e) {
            // Print the exception message and return -100
            std::wcout << e.what() << std::endl;
            return -100;
        }
    }
    // If the command line arguments were not parsed successfully, return 0
    return 0;
}
