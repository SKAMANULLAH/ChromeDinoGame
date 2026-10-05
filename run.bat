@echo off
setlocal
cd /d "%~dp0"

if not exist "build\ChromeDinoRaster.exe" (
    echo [INFO] Executable not found. Building project...
    call build.bat
)

set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%"
echo Starting Chrome Dino Raster Engine...
start "" "%~dp0build\ChromeDinoRaster.exe"
endlocal
