@echo off
setlocal
cd /d "%~dp0"
title Chrome Dino Phone Wi-Fi Server

cls
echo ========================================================================
echo        CHROME DINO: INSTANT PLAY ON YOUR MOBILE PHONE VIA WI-FI
echo ========================================================================
echo.

REM Find the local Wi-Fi IPv4 address
set "LOCAL_IP=localhost"
for /f "tokens=*" %%a in ('powershell -NoProfile -Command "(Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -notmatch 'Loopback|vEthernet' } | Select-Object -ExpandProperty IPAddress)[0]"') do (
    set "LOCAL_IP=%%a"
)

echo   [1] Connect your smartphone to the SAME Wi-Fi network as this PC.
echo.
echo   [2] Open your mobile browser (Chrome, Safari, Brave, Samsung Internet)
echo.
echo   [3] Type this address in your phone's browser address bar:
echo.
echo        ====================================================
echo          👉   http://%LOCAL_IP%:8080
echo        ====================================================
echo.
echo   [4] Turn your phone sideways (Landscape mode) to play!
echo       - Tap Screen or [▲ JUMP] to jump (hold to jump higher)
echo       - Swipe Down or [▼ DUCK] to duck & fast-fall
echo       - Tap [SET] in top-right for Weather, Palettes, CRT & Audio
echo       - Optional: Tap "Add to Home screen" in browser to install!
echo.
echo ========================================================================
echo   Web server is now RUNNING! Keep this window open while playing.
echo   Press Ctrl+C anytime to stop.
echo ========================================================================
echo.

if exist "C:\Users\skama\anaconda3\python.exe" (
    "C:\Users\skama\anaconda3\python.exe" -m http.server 8080 --directory "%~dp0web"
) else (
    python -m http.server 8080 --directory "%~dp0web"
)

endlocal
