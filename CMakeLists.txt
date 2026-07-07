cmake_minimum_required(VERSION 3.16)

project(NwShip VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES NwShip_zh_CN.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
    submarine.ico
    win32.rc
    data/data_loader.cpp
    data/data_loader.h
    core/route_calculator.cpp
    core/route_calculator.h
    widgets/multiselectcombobox.cpp
    widgets/multiselectcombobox.h
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NwShip MANUAL_FINALIZATION ${PROJECT_SOURCES})
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(NwShip SHARED ${PROJECT_SOURCES})
    else()
        add_executable(NwShip ${PROJECT_SOURCES})
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(NwShip PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(NwShip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(NwShip PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(MSVC)
    target_compile_definitions(NwShip PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
    target_sources(NwShip PRIVATE submarine.ico win32.rc)
    set_source_files_properties(submarine.ico PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NwShip)
endif()

foreach(DATA_FILE SubmarineExploration.csv SubmarinePart.csv SubmarineRank.csv)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${DATA_FILE}")
        configure_file(${CMAKE_SOURCE_DIR}/${DATA_FILE}
                       ${CMAKE_BINARY_DIR}/${DATA_FILE} COPYONLY)
    endif()
endforeach()

if(EXISTS "${CMAKE_SOURCE_DIR}/data.bin")
    configure_file(${CMAKE_SOURCE_DIR}/data.bin
                   ${CMAKE_BINARY_DIR}/data.bin COPYONLY)
endif()

if(WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/submarine.ico
                   ${CMAKE_BINARY_DIR}/submarine.ico COPYONLY)
endif()
