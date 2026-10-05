@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   Launching Chrome Dino: Mobile Touch Preview
echo   - Click to Jump (Tap)
echo   - Click ^& Drag Down to Duck (Swipe)
echo   - Click [SET] for In-Game Touch Preferences
echo ========================================================

if exist "deploy\ChromeDinoRaster.exe" (
    start "" /d "%~dp0deploy" "%~dp0deploy\ChromeDinoRaster.exe" --mobile
) else (
    set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%"
    start "" /d "%~dp0build" "%~dp0build\ChromeDinoRaster.exe" --mobile
)

endlocal
