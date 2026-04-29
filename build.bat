@echo off

SET build_simulator=false
SET build_rebuild=false
SET build_copy=false
SET build_flash=false


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
    SET BUILD_FOLDER=%CD%\simulator_build
) else (
    SET BUILD_FOLDER=%CD%\build
)

if "%build_rebuild%"=="true" (
    rd /s /q %BUILD_FOLDER%
) 

if not exist %BUILD_FOLDER% (
    mkdir %BUILD_FOLDER%
)
touch src/core/software_version.c

pushd %BUILD_FOLDER%

del cipher_node_touch.elf
del cipher_node_touch.bin
del cipher_node_touch.hex
del update.bin

cmake .. -G "Ninja"
cmake --build .
if exist cipher_node_touch.bin (
    %CD%/../tools/ota_file_maker/OTA_File_Maker_Console.exe mcu cipher_node_touch.bin update.bin aes
    if "%build_copy%"=="true" (
        copy /b "update.bin" "j:/update.bin"
    )
)

if exist cipher_node_touch.hex (
    if "%build_flash%"=="true" (
        JLink -CommanderScript "../program.jlink"
    )
)

popd

