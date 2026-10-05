@echo off
setlocal
cd /d "%~dp0"
title Chrome Dino Master Launcher

:menu
cls
echo ======================================================================
echo            CHROME DINO: PURE SOFTWARE RASTER ENGINE
echo ======================================================================
echo.
echo   [1] PC Desktop Mode (Default)
echo       * Fullscreen-ready wide display with dark cyberpunk toolbar
echo       * Keyboard controls (Space, Up/Down, F1-F11 hotkeys)
echo.
echo   [2] Mobile Phone Preview (Test on PC)
echo       * Phone screen simulation right on your computer screen!
echo       * On-screen touch pads ([▲ JUMP] and [▼ DUCK])
echo       * Touch gestures: Click/Tap to Jump, Drag Down to Duck
echo.
echo   [3] Play on Your Real Mobile Phone (Instant Wi-Fi Play)
echo       * No installation required!
echo       * Connects your phone over Wi-Fi so you can play on Chrome/Safari
echo.
echo   [4] Native Android APK & Google Play Store Builder
echo       * Packages .apk to install natively on Android phones
echo       * Validates Java JDK, Android SDK/NDK, and Qt compilers
echo.
echo   [5] Rebuild Project from Source
echo.
echo   [6] Exit
echo.
echo ======================================================================
set /p choice="  Choose option [1-6, Default=1]: "

if "%choice%"=="" goto desktop
if "%choice%"=="1" goto desktop
if "%choice%"=="2" goto mobile_preview
if "%choice%"=="3" goto phone_wifi
if "%choice%"=="4" goto android_apk
if "%choice%"=="5" goto rebuild
if "%choice%"=="6" goto quit

echo Invalid option, please try again.
timeout /t 2 >nul
goto menu

:desktop
echo.
echo Launching PC Desktop Mode...
call "PLAY_DINO.bat"
goto quit

:mobile_preview
echo.
echo Launching Mobile Phone Preview Mode...
call "PLAY_MOBILE_MODE.bat"
goto quit

:phone_wifi
echo.
echo Starting Phone Wi-Fi Server...
call "PLAY_ON_PHONE_WIFI.bat"
goto menu

:android_apk
echo.
echo Running Android APK Build Assistant...
call "BUILD_ANDROID_APK.bat"
goto menu

:rebuild
echo.
echo Rebuilding Chrome Dino...
call "build.bat"
if exist "build\ChromeDinoRaster.exe" (
    copy /y "build\ChromeDinoRaster.exe" "deploy\ChromeDinoRaster.exe" >nul
    echo [SUCCESS] Binary updated in deploy folder!
)
pause
goto menu

:quit
endlocal
