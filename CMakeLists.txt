cmake_minimum_required(VERSION 3.16)

project(VTF_Edit_Revitalized CXX)


set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)

if (WIN32)
    set(CMAKE_CXX_STANDARD 23)
else ()
    set(CMAKE_CXX_STANDARD 20)
endif ()

option(CHAOS_INITIATIVE "Build Chaos Specific" OFF)

if (CHAOS_INITIATIVE)
    add_definitions(-DCHAOS_INITIATIVE)
endif ()

option(COLOR_CORRECTION "Build Color Correction (WIP)" OFF)

if (COLOR_CORRECTION)
    add_definitions(-DCOLOR_CORRECTION)
endif ()

option(NORMAL_GENERATION "Build Normal Generation (WIP)" OFF)

if (NORMAL_GENERATION)
    add_definitions(-DNORMAL_GENERATION)
endif ()

if (COMPRESSVTF)
    add_definitions(-DCOMPRESSVTF)
endif ()

# MT/MTd specification for Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "..." FORCE)
endif ()

if (UNIX)
    set(QT_INCLUDE "/usr/include/x86_64-linux-gnu/qt6")
    if (NOT DEFINED QT_BASEDIR)
        set(QT_BASEDIR "$ENV{HOME}/Qt/6.5.3/gcc_64/")
    endif ()
    if (DEFINED QT_BASEDIR)
        set(CMAKE_PREFIX_PATH "${QT_BASEDIR}/lib/cmake")
    else ()
        message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt6/here")
    endif ()
elseif (WIN32)
    # On Windows, the install dir must be specified, as it's not standardized
    if (DEFINED QT_BASEDIR)
        # Add it to the prefix path so find_package can find it
        list(APPEND CMAKE_PREFIX_PATH ${QT_BASEDIR})
        set(QT_INCLUDE ${QT_BASEDIR}/include)
        # CMake has an odd policy that links a special link lib for Qt on newer versions of CMake. Enable it so we don't get spammed, and I get to write less
        cmake_policy(SET CMP0020 NEW)
    else ()
        message(FATAL_ERROR "--!@ Please define your QT install dir with -DQT_BASEDIR=C:/your/qt6/here")
    endif ()
endif ()

message("Using ${QT_INCLUDE} as our Qt include dir")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#find_package(Imath CONFIG)
#find_package(OpenEXR CONFIG REQUIRED)


find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui OpenGL OpenGLWidgets)

set(SRC
        main.cpp
        #        dialogs/VTFEdit.cpp
        #        dialogs/VTFEdit.h
        src/VTFEImport.cpp
        src/VTFEImport.h
        #        widgets/VPKDirectoryHandler.cpp
        #        widgets/VPKDirectoryHandler.h
        src/ImageViewWidget.cpp
        src/ImageViewWidget.h
        src/ResourceWidget.cpp
        src/ResourceWidget.h
        src/enums.cpp
        src/enums.hpp
        src/ImageSettingsWidget.cpp
        src/ImageSettingsWidget.h
        src/flagsandformats.hpp
        src/InfoWidget.cpp
        src/InfoWidget.h
        #        widgets/VTFQMenuBar.cpp
        #        widgets/VTFQMenuBar.h
        #        dialogs/VTFEAbout.cpp
        #        dialogs/VTFEAbout.h
        #        common/VTFEImageFormat.cpp
        src/VTFEImageFormat.h
        libs/stb/stb_image.h
        #        dialogs/VTFEConfigFile.cpp
        #        dialogs/VTFEConfigFile.h
        #        widgets/VMTQSyntaxHighlighter.cpp
        #        widgets/VMTQSyntaxHighlighter.h

        src/MainWindow.cpp src/MainWindow.h
        src/supported_formats/TiffSupport.cpp
        src/supported_formats/TiffSupport.h
        src/Options.cpp)

add_subdirectory(libs/VTFLib)
add_subdirectory(libs/fmt)
#add_subdirectory(libs/QColorWheel)
add_subdirectory(libs/SpeedyKV)

add_executable(${PROJECT_NAME} ${SRC} src/res/res.qrc)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt::OpenGLWidgets OpenGL::GLU vtflib fmt::fmt keyvalues)

target_include_directories(${PROJECT_NAME} PRIVATE "${QT_INCLUDE} ${QT_INCLUDE}/QtWidgets" "${QT_INCLUDE}/QtGui" "${QT_INCLUDE}/QtCore" "${QT_INCLUDE}/QtOpenGLWidgets" ${QT_INCLUDE}/QtOpenGL ${OPENGL_glu_LIBRARY} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} OpenEXR::OpenEXR Imath::Imath Imath::Half)

if (WIN32)
    # Copy these to bundle them with the program in releases
    configure_file("${QT_BASEDIR}/bin/opengl32sw.dll" "${CMAKE_BINARY_DIR}/opengl32sw.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt6Core.dll" "${CMAKE_BINARY_DIR}/Qt6Core.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt6Gui.dll" "${CMAKE_BINARY_DIR}/Qt6Gui.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt6Widgets.dll" "${CMAKE_BINARY_DIR}/Qt6Widgets.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt6OpenGL.dll" "${CMAKE_BINARY_DIR}/Qt6OpenGL.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/bin/Qt6OpenGLWidgets.dll" "${CMAKE_BINARY_DIR}/Qt6OpenGLWidgets.dll" COPYONLY)

    configure_file("${QT_BASEDIR}/plugins/imageformats/qjpeg.dll" "${CMAKE_BINARY_DIR}/imageformats/qjpeg.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/imageformats/qtga.dll" "${CMAKE_BINARY_DIR}/imageformats/qtga.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/imageformats/qwebp.dll" "${CMAKE_BINARY_DIR}/imageformats/qwebp.dll" COPYONLY)

    configure_file("${QT_BASEDIR}/plugins/platforms/qwindows.dll" "${CMAKE_BINARY_DIR}/platforms/qwindows.dll" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/platforms/qwindowsd.dll" "${CMAKE_BINARY_DIR}/platforms/qwindowsd.dll" COPYONLY)
    #We only ever need to ship the debug binaries in debug mode.
    IF (CMAKE_BUILD_TYPE MATCHES Debug)
        configure_file("${QT_BASEDIR}/bin/Qt6Cored.dll" "${CMAKE_BINARY_DIR}/Qt6Cored.dll" COPYONLY)
        configure_file("${QT_BASEDIR}/bin/Qt6Guid.dll" "${CMAKE_BINARY_DIR}/Qt6Guid.dll" COPYONLY)
        configure_file("${QT_BASEDIR}/bin/Qt6Widgetsd.dll" "${CMAKE_BINARY_DIR}/Qt6Widgetsd.dll" COPYONLY)
        configure_file("${QT_BASEDIR}/plugins/platforms/qwindowsd.dll" "${CMAKE_BINARY_DIR}/platforms/qwindowsd.dll" COPYONLY)
    ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
elseif (UNIX)
    configure_file("${QT_BASEDIR}/lib/libQt6Core.so.6.5.3" "${CMAKE_BINARY_DIR}/libQt6Core.so.6" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt6Gui.so.6.5.3" "${CMAKE_BINARY_DIR}/libQt6Gui.so.6" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt6Widgets.so.6.5.3" "${CMAKE_BINARY_DIR}/libQt6Widgets.so.6" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libQt6OpenGLWidgets.so.6" "${CMAKE_BINARY_DIR}/libQt6OpenGLWidgets.so.6" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicui18n.so.56" "${CMAKE_BINARY_DIR}/libicui18n.so.56" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicuuc.so.56" "${CMAKE_BINARY_DIR}/libicuuc.so.56" COPYONLY)
    configure_file("${QT_BASEDIR}/lib/libicudata.so.56" "${CMAKE_BINARY_DIR}/libicudata.so.56" COPYONLY)

    configure_file("${QT_BASEDIR}/plugins/imageformats/libqjpeg.so" "${CMAKE_BINARY_DIR}/imageformats/libqjpeg.so" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/imageformats/libqtga.so" "${CMAKE_BINARY_DIR}/imageformats/libqtga.so" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/imageformats/libqwebp.so" "${CMAKE_BINARY_DIR}/imageformats/libqwebp.so" COPYONLY)

    configure_file("${QT_BASEDIR}/plugins/platforms/libqminimal.so" "${CMAKE_BINARY_DIR}/platforms/libqminimal.so" COPYONLY)
    configure_file("${QT_BASEDIR}/plugins/platforms/libqxcb.so" "${CMAKE_BINARY_DIR}/platforms/libqxcb.so" COPYONLY)
endif ()