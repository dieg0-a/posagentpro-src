cmake_minimum_required(VERSION 3.5)

project(posagent VERSION 0.1 LANGUAGES CXX C)

if (WIN32)
set (CMAKE_PREFIX_PATH "C:\\Qt\\5.12.12\\msvc2017")
include(FetchContent)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf # release-1.3.1
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  libjpeg
  GIT_REPOSITORY https://github.com/winlibs/libjpeg.git
  GIT_TAG        libjpeg-turbo-2.1.0 # release-2.1.0
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  uwebsockets
  GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
  GIT_TAG        v20.62.0 # release-20.62.0
  OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(zlib libjpeg uwebsockets)

find_package(zlib)
find_package(libjpeg)
find_package(uwebsockets)

set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
set(ZLIB_LIBRARY zlib)

endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets PrintSupport REQUIRED)

set(PROJECT_SOURCES
        main.cpp json.cpp printer.cpp json.hpp printer.hpp escpos.cpp
        escpos.hpp base64.hpp jpeg.cpp jpeg.hpp jpeg_load.c httptime.cpp
        httptime.hpp base64.cpp windows_printer.cpp mainwindow.h
        mainwindow.cpp mainwindow.ui about.ui mainwindow.cpp mainwindow.h
        messagesystem.h messagesystem.cpp printercombofieldmodel.h
        printercombofieldmodel.cpp printerdriverlistmodel.h
        printerdriverlistmodel.cpp inputfield.hpp inputfield.cpp windowsprinter.h
        windows_printer_extra.cpp eventclient.cpp eventclient.h eventdata.cpp
        eventdata.h eventsystem.cpp qtprinter.hpp qtprinter.cpp
        eventsystem.h printagent.qrc
)


set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/posagent_resource.rc")

if (WIN32)
   SET(OBJS
#     ${CMAKE_CURRENT_SOURCE_DIR}/winlibs/uSockets.lib
#     ${CMAKE_CURRENT_SOURCE_DIR}/winlibs/uv_a.lib
   )
else()
    SET(OBJS
#      ${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/uSockets.a
    )
endif()

SET_SOURCE_FILES_PROPERTIES(
  ${OBJS}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if (WIN32)
        qt_add_executable(posagent WIN32 ${app_icon_resource_windows} ${OBJS}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    else()
        qt_add_executable(posagent ${OBJS}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    endif()
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PriceEditorCMake APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(posagent ${OBJS} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        if (WIN32)
            add_executable(posagent WIN32 ${app_icon_resource_windows} ${OBJS}
                ${PROJECT_SOURCES}
        )
else()
    add_executable(posagent ${OBJS}
    ${PROJECT_SOURCES}
)

        endif()
    endif()
endif()


#find_package( OpenSSL REQUIRED )
#if ( OPENSSL_FOUND )
#    target_include_directories(posagent PRIVATE ${OPENSSL_INCLUDE_DIRS} )
#    target_link_libraries( posagent ${OPENSSL_LIBRARIES} )
#endif( OPENSSL_FOUND )
if (WIN32)
target_include_directories(posagent PRIVATE ${uwebsockets_SOURCE_DIR}/src ${uwebsockets_SOURCE_DIR}/uSockets/src ${zlib_SOURCE_DIR} ${libjpeg_SOURCE_DIR} ${libjpeg_BINARY_DIR})
else()
    target_include_directories(posagent PRIVATE uWebSockets/src uWebSockets/uSockets/src uWebSockets)
endif()

add_link_options(posagent)
if (WIN32)
   link_directories(${zlib_BINARY_DIR} ${usockets_BINARY_DIR} ${libjpeg_BINARY_DIR} uWebSockets/uSockets/x86win)
else()
   link_directories(uWebSockets/uSockets)
endif()

if (WIN32)
    target_link_libraries(posagent PRIVATE iphlpapi userenv.lib Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport zlib jpeg ${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/x86win/usockets.lib ${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/x86win/uv_a.lib)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
else()
    find_package(ZLIB REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(JPEG 80 REQUIRED)
    target_link_libraries(posagent PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport zlib::zlib OpenSSL::SSL JPEG::JPEG ${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/uSockets.a)
endif()

set_target_properties(posagent PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)


if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20" )
endif()

#ADD_EXECUTABLE(posagent ${OBJS})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(posagent)
endif()

