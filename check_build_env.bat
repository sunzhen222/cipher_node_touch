@echo off
setlocal EnableExtensions EnableDelayedExpansion

chcp 65001 >nul

set REQUIRED_MISSING=0
set RECOMMEND_MISSING=0

echo ============================================================
echo  build.bat 编译环境自查工具
echo ============================================================
echo [信息] 当前目录: %CD%
echo.
echo [一] 检查核心工具

call :check_cmd "cmake" "CMake" "req"
call :check_cmd "ninja" "Ninja" "req"
call :check_cmd "touch" "touch MSYS2 coreutils" "req"
call :check_cmd "arm-none-eabi-gcc" "ARM GCC C 编译器" "req"
call :check_cmd "arm-none-eabi-g++" "ARM GCC C++ 编译器" "req"
call :check_cmd "arm-none-eabi-objcopy" "ARM objcopy" "req"
call :check_cmd "arm-none-eabi-size" "ARM size" "req"
call :check_cmd "powershell" "PowerShell build.bat copy 需要" "req"

echo.
echo [二] 检查建议工具
set "JLINK_PATH="
where /q "JLink"
if errorlevel 1 (
    echo [缺失][建议] SEGGER J-Link 用于 flash - 命令: JLink
    set /a RECOMMEND_MISSING+=1
) else (
    for /f "delims=" %%p in ('where "JLink" 2^>nul') do (
        set "JLINK_PATH=%%p"
        goto :jlink_path_done
    )
    :jlink_path_done
    echo [通过][建议] SEGGER J-Link 用于 flash: 已安装
    echo             路径: !JLINK_PATH!
)

echo.
echo ============================================================
echo  检查结果汇总
echo ============================================================
echo 核心工具缺失: %REQUIRED_MISSING%
echo 建议工具缺失: %RECOMMEND_MISSING%

if not "%REQUIRED_MISSING%"=="0" (
    echo.
    echo [失败] 环境不完整，请先安装缺失的核心工具再执行 build.bat。
    echo        See README.md for Windows build environment setup.
    echo.
    pause
    exit /b 1
)

echo.
echo [通过] build.bat 所需核心工具已齐全。
echo Run: build.bat
echo Run with copy: build.bat copy
echo Run with flash: build.bat flash
echo.
pause
exit /b 0

:check_cmd
set "CMD=%~1"
set "NAME=%~2"
set "LEVEL=%~3"
set "VER_MODE=%~4"
set "LEVEL_ZH=建议"
set "FOUND_PATH="
set "VER_LINE="

if /I "%LEVEL%"=="req" set "LEVEL_ZH=必需"

where /q "%CMD%"
if errorlevel 1 (
    if /I "%LEVEL%"=="req" (
        echo [缺失][必需] !NAME! - 命令: !CMD!
        set /a REQUIRED_MISSING+=1
    ) else (
        echo [缺失][建议] !NAME! - 命令: !CMD!
        set /a RECOMMEND_MISSING+=1
    )
    goto :eof
)

for /f "delims=" %%p in ('where "%CMD%" 2^>nul') do (
    set "FOUND_PATH=%%p"
    goto :check_cmd_path_done
)
:check_cmd_path_done

if /I not "%VER_MODE%"=="noversion" (
    for /f "delims=" %%v in ('"%CMD%" --version 2^>nul') do (
        set "VER_LINE=%%v"
        goto :check_cmd_ver_done
    )
)
:check_cmd_ver_done

if defined VER_LINE (
    echo [通过][!LEVEL_ZH!] !NAME!: !VER_LINE!
) else (
    echo [通过][!LEVEL_ZH!] !NAME!: 已安装
)
echo             路径: !FOUND_PATH!
goto :eof
