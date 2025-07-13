@echo off
echo ==================================
echo CviCubeMX Build Script
echo ==================================

REM 检查是否存在构建目录
if exist "build" (
    echo Cleaning previous build...
    rmdir /s /q build
)

echo Creating build directory...
mkdir build

REM 进入构建目录
cd build

REM 检查Qt环境
set "QT_DIR="
if exist "C:\Qt\6.9.1\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.9.1\msvc2022_64"
if exist "C:\Qt\6.8.0\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.8.0\msvc2022_64"
if exist "C:\Qt\6.7.0\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.7.0\msvc2022_64"

if "%QT_DIR%"=="" (
    echo ERROR: Qt6 not found! Please install Qt6 or update the path in this script.
    echo Looking for Qt in common locations:
    echo   C:\Qt\6.9.1\msvc2022_64\
    echo   C:\Qt\6.8.0\msvc2022_64\
    echo   C:\Qt\6.7.0\msvc2022_64\
    pause
    exit /b 1
)

echo Found Qt at: %QT_DIR%

REM 运行CMake配置
echo Configuring project with CMake...
cmake .. -D CMAKE_BUILD_TYPE=Release -D CMAKE_PREFIX_PATH=%QT_DIR% -D CMAKE_CXX_STANDARD=17 -D CMAKE_CXX_STANDARD_REQUIRED=ON

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

REM 编译项目
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo ==================================
echo Build completed successfully!
echo ==================================

REM 检查是否存在可执行文件
set "EXE_PATH="
if exist "bin\CviCubeMX.exe" (
    set "EXE_PATH=bin\CviCubeMX.exe"
    set "BIN_DIR=bin"
) else if exist "bin\Release\CviCubeMX.exe" (
    set "EXE_PATH=bin\Release\CviCubeMX.exe"
    set "BIN_DIR=bin\Release"
) else if exist "bin\Debug\CviCubeMX.exe" (
    set "EXE_PATH=bin\Debug\CviCubeMX.exe"
    set "BIN_DIR=bin\Debug"
)

if "%EXE_PATH%"=="" (
    echo ERROR: Executable not found!
    echo Searched in:
    echo   - bin\CviCubeMX.exe
    echo   - bin\Release\CviCubeMX.exe
    echo   - bin\Debug\CviCubeMX.exe
    pause
    exit /b 1
)

REM 部署Qt库
echo Deploying Qt libraries...
if exist "%QT_DIR%\bin\windeployqt.exe" (
    "%QT_DIR%\bin\windeployqt.exe" "%EXE_PATH%" --verbose 2
    if %errorlevel% equ 0 (
        echo Qt libraries deployed successfully!
    ) else (
        echo WARNING: windeployqt failed, you may need to manually copy Qt DLLs
    )
) else (
    echo WARNING: windeployqt.exe not found, you may need to manually copy Qt DLLs
    echo Please add %QT_DIR%\bin to your PATH or copy the following DLLs:
    echo   - Qt6Core.dll
    echo   - Qt6Widgets.dll
    echo   - Qt6Gui.dll
)

echo.
echo Executable location: %EXE_PATH%
echo.

REM 询问是否运行程序
set /p run_app="Do you want to run the application now? (y/n): "
if /i "%run_app%"=="y" (
    echo Running CviCubeMX...
    start "%EXE_PATH%"
)

pause
