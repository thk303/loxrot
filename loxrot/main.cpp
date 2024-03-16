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

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void  ServiceMain(int argc, wchar_t** argv);
void  ControlHandler(DWORD request);

struct Args {
    std::wstring configfile = L"";
    std::wstring logfile = PROGRAMNAMEW + L".log";
    int loglevel = Logging::LogLevel::none;
    std::wstring loglevelname = L"none";    // for installing the service
    bool service = false;
    bool foreground = true;
    bool installservice = false;
    bool uninstallservice = false;
};

bool parseArgs(int argc, wchar_t** argv, Args* args) {
    std::wstringstream helptext;
    args->loglevel = Logging::LogLevel::info;
    helptext << PROGRAMNAMEW << L" v" << VERSION << std::endl
        << L"Usage: " + PROGRAMNAMEW + L" --config <configfile> [--foreground] [--logfile <logfile|:stdout>] [--loglevel <loglevel>] [--installservice|--uninstallservice]" << std::endl;
    if (argc < 2) {
        std::wcout << helptext.str() << std::endl;
    }
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"--config") == 0) {
            if (i + 1 < argc) {
                args->configfile = argv[i + 1];
                i++;
            }
            else {
                std::wcout << L"Missing argument for --config" << std::endl;
                return false;
            }
        }
        else if (wcscmp(argv[i], L"--logfile") == 0) {
            if (i + 1 < argc) {
                args->logfile = argv[i + 1];
                i++;
            }
            else {
                std::wcout << L"Missing argument for --logfile" << std::endl;
                return false;
            }
        }
        else if (wcscmp(argv[i], L"--loglevel") == 0) {
            if (i + 1 < argc) {
                args->loglevelname = argv[i + 1];
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
                    std::wcout << L"Wrong argument for --loglevel. Valid values are 'none', 'debug', 'info', 'warning', 'error' and 'fatal'." << std::endl;
                    return false;
                }
            }
            else {
                std::wcout << L"Missing argument for --loglevel. Valid values are 'none', 'debug', 'info', 'warning', 'error' and 'fatal'." << std::endl;
                return false;
            }
            i++;
        }
        else if (wcscmp(argv[i], L"--service") == 0) {
            args->service = true;
        }
        else if (wcscmp(argv[i], L"--foreground") == 0) {
            args->foreground = true;
        }
        else if (wcscmp(argv[i], L"--installservice") == 0) {
            args->installservice = true;
        }
        else if (wcscmp(argv[i], L"--uninstallservice") == 0) {
            args->uninstallservice = true;
        }
        else {
            std::wcout << helptext.str() << std::endl << L"Unknown argument " << argv[i] << std::endl;
            return false;
        }
    }
    return true;
}

void ServiceMain(int argc, wchar_t** argv)
{
    std::wstring cmdline = GetCommandLine();
    int argc_ = 0;
    wchar_t** argv_ = CommandLineToArgvW(cmdline.c_str(), &argc_);
    if (argv_ == NULL) {
		Logging::error(L"CommandLineToArgvW failed");
		return;
	}
    Args args;
    if (parseArgs(argc_, argv_, &args)) {
        Logging::setLogOptions(args.loglevel, args.logfile);

        Config config;
        try {
            config.load(args.configfile);
        }
        catch(std::invalid_argument& e) {
			Logging::fatal(e.what());
            throw std::runtime_error(e.what());
		}
        ServiceStatus.dwServiceType = SERVICE_WIN32;
        ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwServiceSpecificExitCode = 0;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;

        hStatus = RegisterServiceCtrlHandlerW(PROGRAMNAMEW.c_str(), (LPHANDLER_FUNCTION)ControlHandler);

        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(hStatus, &ServiceStatus);

        Rotate rotate;
        while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                rotate.doRotates((std::pair<std::wstring, Config::Section>*)(&(*it)));
            }
            if (config.getConfigs().size() == 0) {
                Logging::error(L"No sections found in config file");
                return;
            }
            Logging::debug(L"Service still running...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        Logging::debug(L"Leaving ServiceMain");
    }
}

void ControlHandler(DWORD request)
{
    switch (request)
    {
    case SERVICE_CONTROL_STOP:
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &ServiceStatus);
        Logging::info(L"Service stopped");
        return;

    case SERVICE_CONTROL_SHUTDOWN:
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &ServiceStatus);
        Logging::info(L"Service shut down");
        return;

    default:
        break;
    }

    SetServiceStatus(hStatus, &ServiceStatus);
}

int wmain(int argc, wchar_t** argv) {
    Args args;
    if(parseArgs(argc, argv, &args)) {
        Logging::setLogOptions(args.loglevel, args.logfile);
        try {
            Logging::info(PROGRAMNAMEW + L" " + VERSION + L" started");
            if (args.installservice) {
                std::wcout << L"Installing service" << std::endl;
                SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
                if (schSCManager) {
                    std::wstring path = L"\"" + std::filesystem::absolute(argv[0]).wstring() + L"\" --service --config " + args.configfile + L" --logfile " + args.logfile + L" --loglevel " + args.loglevelname;
                    SC_HANDLE schService = CreateService(schSCManager, PROGRAMNAMEW.c_str(), PROGRAMNAMEW.c_str(), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path.c_str(), NULL, NULL, NULL, NULL, NULL);
                    if (schService) {
                        Logging::info(L"Service installed");
                        CloseServiceHandle(schService);
                    }
                    else {
                        Logging::error(L"CreateService failed");
                    }
                    CloseServiceHandle(schSCManager);
                }
                else {
                    std::wcout << L"OpenSCManager failed" << std::endl;
                }
            }
            else if (args.uninstallservice) {
                std::wcout << L"Uninstalling service" << std::endl;
                SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
                if (schSCManager) {
                    SC_HANDLE schService = OpenService(schSCManager, PROGRAMNAMEW.c_str(), SERVICE_ALL_ACCESS);
                    if (schService) {
                        if (DeleteService(schService)) {
                            std::wcout << L"Service uninstalled" << std::endl;
                        }
                        else {
                            std::wcout << L"DeleteService failed" << std::endl;
                        }
                        CloseServiceHandle(schService);
                    }
                    else {
                        std::wcout << L"OpenService failed" << std::endl;
                    }
                }
            }
            else {
                if (args.service) {
                    Logging::info(L"Starting as service");
                    SERVICE_TABLE_ENTRY ServiceTable[2];
                    ServiceTable[0].lpServiceName = (LPWSTR)PROGRAMNAME.c_str();
                    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONW)ServiceMain;

                    ServiceTable[1].lpServiceName = NULL;
                    ServiceTable[1].lpServiceProc = NULL;

                    StartServiceCtrlDispatcher(ServiceTable);
                    Logging::debug(L"Service after StartServiceCtrlDispatcher");
                }
                else {
                    Logging::info(L"Starting as console application");
                    Config config;
                    try {
                        config.load(args.configfile);
                    }
                    catch (std::invalid_argument& e) {
                        Logging::fatal(e.what());
                        throw std::runtime_error(e.what());
                    }
                    if (config.getConfigs().size() == 0) {
                        Logging::fatal(L"No sections found in config file");
                        throw std::runtime_error("No sections found in config file");
                    }
                    for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                        Logging::info(L"Checking " + it->first);
                    }
                    Rotate rotate;
                    while (1) {
                        for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
                            rotate.doRotates((std::pair<std::wstring, Config::Section>*)(&(*it)));
                        }
                        if (args.foreground) {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                        else {
                            break;
                        }

                    }
                    Logging::info(PROGRAMNAMEW + L" " + VERSION + L" finished");
                }
            }
            return 0;
        }
        catch (std::runtime_error& e) {
            std::wcout << e.what() << std::endl;
            return -100;
        }
    }
    return 0;
}
