@echo off
setlocal

set "PORT=%~1"
if "%PORT%"=="" set "PORT=COM11"

set "COMMANDS_FILE=%~2"
if "%COMMANDS_FILE%"=="" set "COMMANDS_FILE=.\tools\serial_test_plan.json"

set "START_PATTERN=%~3"

call "%~dp0..\build.bat" flash
if errorlevel 1 (
    echo [ERROR] Flash failed.
    exit /b 1
)

if "%START_PATTERN%"=="" (
    powershell -ExecutionPolicy Bypass -File "%~dp0serial_test.ps1" -Port "%PORT%" -BaudRate 921600 -CommandsFile "%COMMANDS_FILE%"
) else (
    powershell -ExecutionPolicy Bypass -File "%~dp0serial_test.ps1" -Port "%PORT%" -BaudRate 921600 -CommandsFile "%COMMANDS_FILE%" -StartPattern "%START_PATTERN%"
)
if errorlevel 1 (
    echo [ERROR] Serial test failed.
    exit /b 1
)

echo [OK] Flash and serial test completed.
exit /b 0
