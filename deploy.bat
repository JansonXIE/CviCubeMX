@echo off
echo ==================================
echo CviCubeMX Deployment Script
echo ==================================

REM 检查是否存在可执行文件
set "EXE_PATH="
if exist "build\bin\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\CviCubeMX.exe"
) else if exist "build\bin\Release\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\Release\CviCubeMX.exe"
) else if exist "build\bin\Debug\CviCubeMX.exe" (
    set "EXE_PATH=build\bin\Debug\CviCubeMX.exe"
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

REM 查找Qt安装目录
set "QT_DIR="
if exist "C:\Qt\6.9.1\msvc2022_64\bin\windeployqt.exe" set "QT_DIR=C:\Qt\6.9.1\msvc2022_64"
if exist "C:\Qt\6.8.0\msvc2022_64\bin\windeployqt.exe" set "QT_DIR=C:\Qt\6.8.0\msvc2022_64"
if exist "C:\Qt\6.7.0\msvc2022_64\bin\windeployqt.exe" set "QT_DIR=C:\Qt\6.7.0\msvc2022_64"

if "%QT_DIR%"=="" (
    echo ERROR: windeployqt.exe not found!
    echo Please ensure Qt6 is installed and update the path in this script.
    echo.
    echo Manual deployment steps:
    echo 1. Copy the following DLLs from Qt bin directory to build\bin\:
    echo    - Qt6Core.dll
    echo    - Qt6Widgets.dll
    echo    - Qt6Gui.dll
    echo    - Qt6Network.dll (if needed)
    echo.
    echo 2. Copy platforms folder from Qt plugins directory
    echo 3. Copy styles folder from Qt plugins directory (if needed)
    pause
    exit /b 1
)

echo Found Qt at: %QT_DIR%

REM 创建便携版目录
set "PORTABLE_DIR=CviCubeMX_Portable"
if exist "%PORTABLE_DIR%" (
    echo Cleaning previous portable version...
    rmdir /s /q "%PORTABLE_DIR%"
)

echo Creating portable version...
mkdir "%PORTABLE_DIR%"

REM 复制可执行文件
copy "%EXE_PATH%" "%PORTABLE_DIR%\"

REM 使用windeployqt部署Qt库
echo Deploying Qt libraries...
"%QT_DIR%\bin\windeployqt.exe" "%PORTABLE_DIR%\CviCubeMX.exe" --verbose 2 --release --no-translations --no-system-d3d-compiler --no-opengl-sw

if %errorlevel% equ 0 (
    echo ==================================
    echo Deployment completed successfully!
    echo ==================================
    echo.
    echo Portable version created in: %PORTABLE_DIR%\
    echo You can copy this folder to any Windows computer and run CviCubeMX.exe
    echo.
    
    REM 创建启动脚本
    echo @echo off > "%PORTABLE_DIR%\Run_CviCubeMX.bat"
    echo echo Starting CviCubeMX... >> "%PORTABLE_DIR%\Run_CviCubeMX.bat"
    echo start CviCubeMX.exe >> "%PORTABLE_DIR%\Run_CviCubeMX.bat"
    
    echo Created Run_CviCubeMX.bat for easy launching.
    echo.
    
    REM 询问是否测试运行
    set /p test_run="Do you want to test the portable version now? (y/n): "
    if /i "%test_run%"=="y" (
        echo Testing portable version...
        start "%PORTABLE_DIR%\CviCubeMX.exe"
    )
) else (
    echo ERROR: Deployment failed!
    echo Please check the error messages above.
)

pause
