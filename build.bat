@echo off

SET build_simulator=false
SET build_rebuild=false
SET build_copy=false


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

REM Generate i18n YML files from CSV
echo Generating i18n files from CSV...
python tools/generate_i18n.py

call npx lv_i18n compile -t "src/ui/i18n/en.yml" -t "src/ui/i18n/zh-cn.yml" -t "src/ui/i18n/ko.yml" -o "src/ui/i18n"

REM Generate mixed font (English + CJK)
echo Generating mixed font...
set "CJK_CHARS_FILE=%TEMP%\cjk_chars.tmp"
python tools/extract_chinese_chars.py src/ui/i18n/zh-cn.yml src/ui/i18n/ko.yml > "%CJK_CHARS_FILE%"
if errorlevel 1 exit /b 1
python tools/generate_mixed_font.py "%CJK_CHARS_FILE%" src/ui/fonts/lv_font_mix_14.c
del "%CJK_CHARS_FILE%" >nul 2>&1
if errorlevel 1 exit /b 1
echo Mixed font generated successfully.

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

popd

