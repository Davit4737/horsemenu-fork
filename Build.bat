@echo off
setlocal
cd /d "%~dp0"

echo ===== Building HorseMenu =====
echo Project: %CD%
echo.

where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: cmake not found.
    echo Install Visual Studio 2022 with "Desktop development with C++".
    goto fail
)

echo [1/2] Configure...
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 goto fail

echo.
echo [2/2] Build Release (single-threaded)...
cmake --build build --config Release -- /m:1 /v:minimal
if errorlevel 1 (
    echo.
    echo Retrying with normal verbosity...
    cmake --build build --config Release -- /m:1 /v:normal
    if errorlevel 1 goto fail
)

set "DLL=%~dp0build\Release\HorseMenu.dll"
if exist "%DLL%" (
    echo.
    echo ===== BUILD SUCCESS =====
    echo DLL: %DLL%
    echo.
    explorer /select,"%DLL%"
    pause
    exit /b 0
)

echo.
echo Build finished but DLL missing:
echo %DLL%
goto fail

:fail
echo.
echo ===== Build failed =====
echo.
echo IMPORTANT: Run this from the project folder, NOT from inside build\
echo   Correct:  C:\Users\user\Downloads\HorseMenu-nightly\Build.bat
echo   Wrong:    building from build\build\
echo.
echo Tips:
echo  - Normal PowerShell is fine if cmake works (ignore DevShell errors)
echo  - If errors persist, delete the entire "build" folder and run Build.bat again
echo.
pause
exit /b 1
