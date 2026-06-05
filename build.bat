@echo off
setlocal EnableDelayedExpansion

SET SCRIPT_DIR=%~dp0
SET SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
SET build_simulator=false
SET build_rebuild=false
SET build_copy=false
SET build_flash=false
SET target_min_bytes=768000
SET target_max_bytes=819200


for %%i in (%*) do (
    if /I "%%i"=="simulator" (
        set build_simulator=true
    )
    if /I "%%i"=="rebuild" (
        set build_rebuild=true
    )
    if /I "%%i"=="copy" (
        set build_copy=true
    )
    if /I "%%i"=="flash" (
        set build_flash=true
    )
)

if "%build_simulator%"=="true" (
    SET BUILD_FOLDER=%SCRIPT_DIR%\simulator_build
) else (
    SET BUILD_FOLDER=%SCRIPT_DIR%\build
)

if "%build_rebuild%"=="true" (
    rd /s /q %BUILD_FOLDER%
) 

if not exist %BUILD_FOLDER% (
    mkdir %BUILD_FOLDER%
)
touch "%SCRIPT_DIR%\src\core\software_version.c"

pushd %BUILD_FOLDER%

del /q cipher_node_touch.elf >nul 2>&1
del /q cipher_node_touch.bin >nul 2>&1
del /q cipher_node_touch.hex >nul 2>&1
del /q update.bin >nul 2>&1

cmake .. -G "Ninja"
cmake --build .
if exist cipher_node_touch.bin (
    copy /Y /B "%SCRIPT_DIR%\tools\ota_file_maker\key.json" "key.json"
    "%SCRIPT_DIR%\tools\ota_file_maker\OTA_File_Maker_Console.exe" mcu cipher_node_touch.bin update.bin aes
    if "%build_copy%"=="true" (
        set target_drive=
        for /f "usebackq delims=" %%d in (`powershell -NoProfile -Command "$min=%target_min_bytes%;$max=%target_max_bytes%;Get-CimInstance Win32_LogicalDisk | Where-Object { $_.DriveType -in 2,3 -and $_.Size -ge $min -and $_.Size -le $max } | Select-Object -ExpandProperty DeviceID -First 1"`) do set target_drive=%%d
        if defined target_drive (
            echo Copy update.bin to !target_drive!\update.bin
            copy /b "update.bin" "!target_drive!\update.bin"
        ) else (
            echo [WARN] No drive found in !target_min_bytes!~!target_max_bytes! bytes. Skip copy.
        )
    )
)

if exist cipher_node_touch.hex (
    if "%build_flash%"=="true" (
        JLink -CommanderScript "../program.jlink"
    )
)

popd
endlocal

