@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ========================================================
echo   Automated Android APK Build Pipeline
echo ========================================================
echo.

set "SDK_ROOT=C:\Users\skama\AppData\Local\Android\Sdk"
set "BUILD_TOOLS=%SDK_ROOT%\build-tools\34.0.0"
set "PLATFORM=%SDK_ROOT%\platforms\android-34\android.jar"
set "JAVA_BIN=C:\Program Files\Android\Android Studio\jbr\bin"
set "PROJECT_DIR=%~dp0android-apk"

if not exist "%PLATFORM%" (
    echo [ERROR] Android platform 34 not found at: %PLATFORM%
    pause
    exit /b 1
)

echo [1/7] Copying latest game assets...
copy /y "%~dp0web\index.html" "%PROJECT_DIR%\assets\index.html" >nul

echo [2/7] Compiling Android resources with AAPT2...
"%BUILD_TOOLS%\aapt2.exe" compile --dir "%PROJECT_DIR%\res" -o "%PROJECT_DIR%\compiled_res.zip"
if %ERRORLEVEL% neq 0 ( echo [ERROR] AAPT2 compile failed & pause & exit /b 1 )

echo [3/7] Linking resources and package...
if not exist "%PROJECT_DIR%\gen" mkdir "%PROJECT_DIR%\gen"
if not exist "%PROJECT_DIR%\bin" mkdir "%PROJECT_DIR%\bin"
"%BUILD_TOOLS%\aapt2.exe" link "%PROJECT_DIR%\compiled_res.zip" -I "%PLATFORM%" --manifest "%PROJECT_DIR%\AndroidManifest.xml" -o "%PROJECT_DIR%\unaligned.apk" --java "%PROJECT_DIR%\gen" -A "%PROJECT_DIR%\assets" --auto-add-overlay
if %ERRORLEVEL% neq 0 ( echo [ERROR] AAPT2 link failed & pause & exit /b 1 )

echo [4/7] Compiling Java code...
"%JAVA_BIN%\javac.exe" -cp "%PLATFORM%" -d "%PROJECT_DIR%\bin" "%PROJECT_DIR%\gen\com\cg\chromedino\R.java" "%PROJECT_DIR%\src\com\cg\chromedino\MainActivity.java"
if %ERRORLEVEL% neq 0 ( echo [ERROR] Java compilation failed & pause & exit /b 1 )

echo [5/7] Converting to Dalvik Executable (classes.dex)...
set "CLASS_FILES="
for /r "%PROJECT_DIR%\bin" %%f in (*.class) do (
    set "CLASS_FILES=!CLASS_FILES! "%%f""
)
call "%BUILD_TOOLS%\d8.bat" !CLASS_FILES! --lib "%PLATFORM%" --output "%PROJECT_DIR%"
if %ERRORLEVEL% neq 0 ( echo [ERROR] D8 compilation failed & pause & exit /b 1 )

echo [6/7] Packaging classes.dex and aligning APK...
"%JAVA_BIN%\jar.exe" uf "%PROJECT_DIR%\unaligned.apk" -C "%PROJECT_DIR%" classes.dex
"%BUILD_TOOLS%\zipalign.exe" -p -f 4 "%PROJECT_DIR%\unaligned.apk" "%PROJECT_DIR%\aligned.apk"
if %ERRORLEVEL% neq 0 ( echo [ERROR] Zipalign failed & pause & exit /b 1 )

echo [7/7] Signing APK with Android Debug Keystore...
if not exist "%PROJECT_DIR%\debug.keystore" (
    "%JAVA_BIN%\keytool.exe" -genkey -v -keystore "%PROJECT_DIR%\debug.keystore" -alias androiddebugkey -storepass android -keypass android -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Android Debug,O=Android,C=US"
)
call "%BUILD_TOOLS%\apksigner.bat" sign --ks "%PROJECT_DIR%\debug.keystore" --ks-pass pass:android --out "%~dp0ChromeDino.apk" "%PROJECT_DIR%\aligned.apk"
if %ERRORLEVEL% neq 0 ( echo [ERROR] APK signing failed & pause & exit /b 1 )

REM Copy to Windows Desktop for instant sharing
set "DESK=%USERPROFILE%\Desktop"
if exist "%DESK%" copy /y "%~dp0ChromeDino.apk" "%DESK%\ChromeDino.apk" >nul

echo.
echo ========================================================
echo   [SUCCESS] Native Android APK built and verified!
echo ========================================================
echo.
echo   File generated at:
echo     1. %~dp0ChromeDino.apk
echo     2. On your Desktop: ChromeDino.apk
echo.
echo   How to send to your phone:
echo     1. Open WhatsApp Web on your PC.
echo     2. Drag and drop ChromeDino.apk into your chat.
echo     3. On your phone, tap the file and click "Install"!
echo ========================================================
echo.
pause
endlocal
