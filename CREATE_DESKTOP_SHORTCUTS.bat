@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   Creating Windows Desktop Shortcuts for Chrome Dino
echo ========================================================
echo.

powershell -NoProfile -Command "$ws = New-Object -ComObject WScript.Shell; $desk = [Environment]::GetFolderPath('Desktop'); $s1 = $ws.CreateShortcut([System.IO.Path]::Combine($desk, 'Chrome Dino (PC Desktop).lnk')); $s1.TargetPath = '%~dp0PLAY_DESKTOP_PC.bat'; $s1.WorkingDirectory = '%~dp0'; $s1.Save(); $s2 = $ws.CreateShortcut([System.IO.Path]::Combine($desk, 'Chrome Dino (Mobile Preview).lnk')); $s2.TargetPath = '%~dp0PLAY_MOBILE_MODE.bat'; $s2.WorkingDirectory = '%~dp0'; $s2.Save(); $s3 = $ws.CreateShortcut([System.IO.Path]::Combine($desk, 'Chrome Dino (Play on Phone via WiFi).lnk')); $s3.TargetPath = '%~dp0PLAY_ON_PHONE_WIFI.bat'; $s3.WorkingDirectory = '%~dp0'; $s3.Save()"

if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Shortcuts created on your Desktop:
    echo   1. "Chrome Dino (PC Desktop)"
    echo   2. "Chrome Dino (Mobile Preview)"
    echo   3. "Chrome Dino (Play on Phone via WiFi)"
) else (
    echo [NOTICE] Could not automatically create desktop shortcuts.
)

echo.
pause
endlocal
