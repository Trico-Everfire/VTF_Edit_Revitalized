cmake_minimum_required(VERSION 3.16)

project(QColorWheel CXX)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE)
endif ()

if (UNIX)
    set(QT_INCLUDE "/usr/include/x86_64-linux-gnu/qt5")
elseif (WIN32)
    # On Windows, the install dir must be specified, as it's not standardized
    if (DEFINED QT_BASEDIR)
        # Add it to the prefix path so find_package can find it
        list(APPEND CMAKE_PREFIX_PATH ${QT_BASEDIR})
        set(QT_INCLUDE ${QT_BASEDIR}/include)
        # CMake has an odd policy that links a special link lib for Qt on newer versions of CMake. Enable it so we don't get spammed, and I get to write less
        cmake_policy(SET CMP0020 NEW)
    else ()
        message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt5/here")
    endif ()
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

add_library(${PROJECT_NAME} SHARED QtColorTriangle.cpp QtColorTriangle.h)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)