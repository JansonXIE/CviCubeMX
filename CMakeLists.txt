cmake_minimum_required(VERSION 3.16)
project(CviCubeMX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/chipconfig.cpp
    src/pinwidget.cpp
    src/codegenerator.cpp
    src/pinfunction.cpp
    src/dtsconfig.cpp
    src/peripheralconfigdialog.cpp
    src/clockconfig.cpp
    src/memoryconfig.cpp
)

set(HEADERS
    src/mainwindow.h
    src/chipconfig.h
    src/pinwidget.h
    src/codegenerator.h
    src/dtsconfig.h
    src/peripheralconfigdialog.h
    src/clockconfig.h
    src/memoryconfig.h
    include/pinfunction.h
    include/cvi_board_init.h
)

set(UI_FILES
    src/mainwindow.ui
)

qt_add_executable(CviCubeMX ${SOURCES} ${HEADERS})
qt_add_resources(CviCubeMX "resources" FILES resources/style.qss)
qt_add_resources(CviCubeMX "icons" FILES
    resources/icons/check_black.svg
    resources/icons/check_white.svg
    resources/icons/blank_20.svg
)

target_include_directories(CviCubeMX PRIVATE include)
target_link_libraries(CviCubeMX
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

# Copy output to bin directory
set_target_properties(CviCubeMX PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
