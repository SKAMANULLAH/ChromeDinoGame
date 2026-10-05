@echo off
setlocal
cd /d "%~dp0"

set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%"

if exist "build\ChromeDinoMini.exe" (
    start "" "build\ChromeDinoMini.exe"
) else (
    echo ChromeDinoMini.exe not built yet. Running build_mini.bat...
    call build_mini.bat
)

endlocal
