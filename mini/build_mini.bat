@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   Building Chrome Dino (Mini Qt Version - Milestone 1)
echo ========================================================

set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%"

cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

cmake --build build
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] ChromeDinoMini built successfully!
echo Launching...
start "" "build\ChromeDinoMini.exe"

endlocal
