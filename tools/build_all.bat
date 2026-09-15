@echo off
rem SanDaoPoker one-click rebuild: locates Visual Studio via vswhere, then configures and builds.
rem Usage: double-click this file, or run it from any directory.
cd /d "%~dp0.."

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe not found. Install Visual Studio 2022 with "Desktop development with C++".
    pause
    exit /b 1
)

set "VSDIR="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
    echo [ERROR] Visual Studio C++ toolchain not found.
    pause
    exit /b 1
)

call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment.
    pause
    exit /b 1
)

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    pause
    exit /b 1
)

cmake --build build
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo.
echo Build finished. Run bin\SanDaoPoker.exe
pause
exit /b 0
