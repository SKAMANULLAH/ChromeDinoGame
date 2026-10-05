@echo off
setlocal
echo ========================================================
echo   Building Chrome Dino (Computer Graphics Raster Lab)
echo ========================================================

set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%"

if not exist build (
    mkdir build
)

cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    exit /b %ERRORLEVEL%
)

cmake --build build
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] ChromeDinoRaster built successfully!
echo Run 'run.bat' to launch the game and CG algorithm lab.
endlocal
