@echo off
setlocal enabledelayedexpansion
echo ========================================================
echo   Building Chrome Dino (Pure Software Raster Engine)
echo ========================================================

:: Check if cmake is in system PATH
where cmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [INFO] CMake found in system PATH.
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    if !ERRORLEVEL! equ 0 (
        cmake --build build --config Release
        if !ERRORLEVEL! equ 0 (
            echo.
            echo [SUCCESS] ChromeDinoRaster built successfully!
            echo Launching game...
            start "" "build\ChromeDinoRaster.exe"
            exit /b 0
        )
    )
)

:: If system CMake failed or not found, try common Qt installation paths
echo [INFO] Checking local Qt installations...
set "FOUND_QT="
for %%P in (
    "C:\Qt\6.11.1\mingw_64"
    "C:\Qt\6.10.0\mingw_64"
    "C:\Qt\6.9.0\mingw_64"
    "C:\Qt\6.8.0\mingw_64"
    "C:\Qt\6.7.0\mingw_64"
    "C:\Qt\6.6.0\mingw_64"
    "C:\Qt\6.5.0\mingw_64"
) do (
    if exist "%%~P" (
        set "FOUND_QT=%%~P"
        goto :found_qt
    )
)

:found_qt
if defined FOUND_QT (
    echo [INFO] Found Qt at %FOUND_QT%
    set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;%FOUND_QT%\bin;%PATH%"
    cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="%FOUND_QT%" -DCMAKE_BUILD_TYPE=Release
    if !ERRORLEVEL! neq 0 (
        echo [ERROR] CMake configuration failed!
        exit /b !ERRORLEVEL!
    )
    cmake --build build
    if !ERRORLEVEL! neq 0 (
        echo [ERROR] Compilation failed!
        exit /b !ERRORLEVEL!
    )
    echo.
    echo [SUCCESS] ChromeDinoRaster built successfully!
    echo Launching game...
    start "" "build\ChromeDinoRaster.exe"
) else (
    echo.
    echo [NOTE] Qt 6 development environment not found in PATH or C:\Qt.
    echo To play instantly without compiling, simply double-click 'ChromeDino.exe'
    echo or open 'web\index.html' in your browser!
)

endlocal
