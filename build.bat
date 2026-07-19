@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo Visual Studio Installer not found.
    exit /b 1
)

for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALLDIR=%%I"

if not defined VSINSTALLDIR (
    echo Visual Studio with C++ tools was not found.
    exit /b 1
)

set "MSVC_ROOT=%VSINSTALLDIR%\VC\Tools\MSVC"
set "MSVC_BIN="
for /d %%D in ("%MSVC_ROOT%\*") do (
    if exist "%%~fD\bin\Hostx64\x64\cl.exe" (
        set "MSVC_BIN=%%~fD\bin\Hostx64\x64"
        set "MSVC_INCLUDE=%%~fD\include"
        set "MSVC_LIB=%%~fD\lib\x64"
        goto :found_msvc
    )
)

:found_msvc
if not defined MSVC_BIN (
    echo Could not locate the MSVC compiler.
    exit /b 1
)

set "SDK_ROOT=%ProgramFiles(x86)%\Windows Kits\10"
set "SDK_INCLUDE="
for /f "usebackq delims=" %%I in (`dir /b /ad "%SDK_ROOT%\Include" 2^>nul ^| sort /R`) do (
    if exist "%SDK_ROOT%\Include\%%I\ucrt" (
        set "SDK_INCLUDE=%SDK_ROOT%\Include\%%I"
        goto :found_sdk_include
    )
)

:found_sdk_include
if not defined SDK_INCLUDE (
    echo Could not locate the Windows SDK headers.
    exit /b 1
)

set "SDK_LIB="
for /f "usebackq delims=" %%I in (`dir /b /ad "%SDK_ROOT%\Lib" 2^>nul ^| sort /R`) do (
    if exist "%SDK_ROOT%\Lib\%%I\ucrt\x64" (
        set "SDK_LIB=%SDK_ROOT%\Lib\%%I"
        goto :found_sdk_lib
    )
)

:found_sdk_lib
if not defined SDK_LIB (
    echo Could not locate the Windows SDK libraries.
    exit /b 1
)

set "INCLUDE=%MSVC_INCLUDE%;%SDK_INCLUDE%\ucrt;%SDK_INCLUDE%\um;%SDK_INCLUDE%\shared%"
set "LIB=%MSVC_LIB%;%SDK_LIB%\ucrt\x64;%SDK_LIB%\um\x64"
set "PATH=%MSVC_BIN%;%PATH%"

"%MSVC_BIN%\cl.exe" /nologo /Zi /EHsc /Fe:"%SCRIPT_DIR%FP.exe" "%SCRIPT_DIR%FP.c"
exit /b %ERRORLEVEL%