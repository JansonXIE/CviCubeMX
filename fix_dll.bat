@echo off
echo ==================================
echo CviCubeMX DLL Fix Script
echo ==================================
echo.
echo This script will help fix the "Qt6Widgets.dll not found" error.
echo.

REM 检查是否存在可执行文件
set "EXE_PATH="
if exist "build\bin\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\CviCubeMX.exe"
    set "BIN_DIR=build\bin"
) else if exist "build\bin\Release\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\Release\CviCubeMX.exe"
    set "BIN_DIR=build\bin\Release"
) else if exist "build\bin\Debug\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\Debug\CviCubeMX.exe"
    set "BIN_DIR=build\bin\Debug"
)

if "%EXE_PATH%"=="" (
    echo ERROR: CviCubeMX.exe not found!
    echo Searched in:
    echo   - build\bin\CviCubeMX.exe
    echo   - build\bin\Release\CviCubeMX.exe
    echo   - build\bin\Debug\CviCubeMX.exe
    echo.
    echo Please build the project first using build.bat
    pause
    exit /b 1
)

echo Found CviCubeMX.exe at: %EXE_PATH%
echo.

REM 查找Qt安装目录
echo Searching for Qt installation...
set "QT_FOUND=0"

if exist "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Core.dll" (
    set "QT_DIR=C:\Qt\6.9.1\msvc2022_64"
    set "QT_FOUND=1"
    echo Found Qt 6.9.1 at: %QT_DIR%
)

if "%QT_FOUND%"=="0" if exist "C:\Qt\6.8.0\msvc2022_64\bin\Qt6Core.dll" (
    set "QT_DIR=C:\Qt\6.8.0\msvc2022_64"
    set "QT_FOUND=1"
    echo Found Qt 6.8.0 at: %QT_DIR%
)

if "%QT_FOUND%"=="0" if exist "C:\Qt\6.7.0\msvc2022_64\bin\Qt6Core.dll" (
    set "QT_DIR=C:\Qt\6.7.0\msvc2022_64"
    set "QT_FOUND=1"
    echo Found Qt 6.7.0 at: %QT_DIR%
)

if "%QT_FOUND%"=="0" (
    echo WARNING: Qt installation not found in common locations.
    echo.
    echo Please manually locate your Qt installation and copy the following files
    echo from Qt_Installation\bin\ to build\bin\:
    echo.
    echo Required DLL files:
    echo   - Qt6Core.dll
    echo   - Qt6Widgets.dll
    echo   - Qt6Gui.dll
    echo.
    echo Optional (copy if needed):
    echo   - Qt6Network.dll
    echo   - msvcp140.dll
    echo   - vcruntime140.dll
    echo   - vcruntime140_1.dll
    echo.
    pause
    exit /b 1
)

echo.
echo Copying Qt DLL files...

REM 复制必需的DLL文件
copy "%QT_DIR%\bin\Qt6Core.dll" "%BIN_DIR%\" >nul 2>&1
copy "%QT_DIR%\bin\Qt6Widgets.dll" "%BIN_DIR%\" >nul 2>&1
copy "%QT_DIR%\bin\Qt6Gui.dll" "%BIN_DIR%\" >nul 2>&1

REM 检查复制是否成功
if exist "%BIN_DIR%\Qt6Core.dll" if exist "%BIN_DIR%\Qt6Widgets.dll" if exist "%BIN_DIR%\Qt6Gui.dll" (
    echo ✓ Successfully copied Qt6Core.dll
    echo ✓ Successfully copied Qt6Widgets.dll  
    echo ✓ Successfully copied Qt6Gui.dll
) else (
    echo ERROR: Failed to copy some DLL files!
    echo Please check file permissions and try running as administrator.
    pause
    exit /b 1
)

REM 复制platforms插件目录
if exist "%QT_DIR%\plugins\platforms" (
    echo Copying Qt platforms plugins...
    if not exist "%BIN_DIR%\platforms" mkdir "%BIN_DIR%\platforms"
    copy "%QT_DIR%\plugins\platforms\qwindows.dll" "%BIN_DIR%\platforms\" >nul 2>&1
    if exist "%BIN_DIR%\platforms\qwindows.dll" (
        echo ✓ Successfully copied platforms plugin
    )
)

echo.
echo ==================================
echo DLL Fix completed!
echo ==================================
echo.
echo Now you should be able to run CviCubeMX.exe without DLL errors.
echo.

REM 询问是否测试运行
set /p test_run="Do you want to test run the application now? (y/n): "
if /i "%test_run%"=="y" (
    echo Starting CviCubeMX...
    cd "%BIN_DIR%"
    start CviCubeMX.exe
    cd ..\..
    if "%BIN_DIR%"=="build\bin\Release" cd ..
) else (
    echo You can now run: %EXE_PATH%
)

echo.
echo If you still encounter DLL errors, please:
echo 1. Install Visual C++ Redistributable 2022
echo 2. Try running deploy.bat for a complete deployment
echo 3. Check Windows Event Viewer for detailed error information
echo.

pause
