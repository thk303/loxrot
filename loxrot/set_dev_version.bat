@echo off
setlocal

:: Git-Befehle, um den aktuellen Tag, den Branch und die Commit-Referenznummer zu erhalten
for /f "tokens=* delims= " %%a in ('date /T') do set CURRENT_DATE=%%a
for /f "tokens=* delims= " %%a in ('git rev-parse --abbrev-ref HEAD') do set CURRENT_BRANCH=%%a
for /f "tokens=* delims= " %%a in ('git rev-parse --short HEAD') do set CURRENT_COMMIT=%%a

:: Erstellen Sie die neue Version
set NEW_VERSION=%CURRENT_COMMIT%_%CURRENT_BRANCH% %CURRENT_DATE%

:: Ersetzen Sie die alte Version durch die neue Version in der Datei version.h
powershell -Command "(gc version.h) -replace 'const std::wstring VERSION = L\".*\";', 'const std::wstring VERSION = L\"%NEW_VERSION%\";' | Out-File -encoding ASCII version.h"

endlocal
