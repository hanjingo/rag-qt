cmake_minimum_required(VERSION 3.16)

# ----------------------------------------------------------------------------
# Qt
# ----------------------------------------------------------------------------
project(rag-qt VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_RELEASE_LITE "Build release lite model" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Multimedia)
find_package(hj REQUIRED)
find_package(whisper REQUIRED)

# for private includes (zip, ...)
set(Qt_PRIVATE_INCLUDES 
    ${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS}
    ${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS}
)


# ----------------------------------------------------------------------------
# CMake Policies
# ----------------------------------------------------------------------------
# CMP0168: Modern FetchContent integration
if(POLICY CMP0168)
    cmake_policy(SET CMP0168 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0168 NEW)
endif()

# CMP0141: MSVC debug information format
cmake_policy(SET CMP0141 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0141 NEW)

# CMP0075: Include files in CheckIncludeFile
cmake_policy(SET CMP0075 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

# CMP0194: MSVC is not an assembler for language ASM (zstd)
if(POLICY CMP0194)
    cmake_policy(SET CMP0194 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0194 NEW)
endif()

# ----------------------------------------------------------------------------
# translation
# ----------------------------------------------------------------------------
file(GLOB TS_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/res/languages/*.ts")

if(TARGET Qt${QT_VERSION_MAJOR}::lrelease)
    set(LRELEASE_EXECUTABLE "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::lrelease>")
else()
    find_program(LRELEASE_EXECUTABLE NAMES lrelease-qt6 lrelease-qt5 lrelease)
    if(NOT LRELEASE_EXECUTABLE)
        message(FATAL_ERROR "Could not find Qt lrelease executable")
    endif()
endif()

set(QM_FILES)
foreach(TS_FILE IN LISTS TS_FILES)
    get_filename_component(TS_DIR "${TS_FILE}" DIRECTORY)
    get_filename_component(TS_NAME_WE "${TS_FILE}" NAME_WE)
    set(QM_BUILD_FILE "${CMAKE_CURRENT_BINARY_DIR}/res/languages/${TS_NAME_WE}.qm")
    set(QM_FILE "${TS_DIR}/${TS_NAME_WE}.qm")

    add_custom_command(
        OUTPUT "${QM_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/res/languages"
        COMMAND "${LRELEASE_EXECUTABLE}" "${TS_FILE}" -qm "${QM_BUILD_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${QM_BUILD_FILE}" "${QM_FILE}"
        DEPENDS "${TS_FILE}"
        COMMENT "Generating ${TS_NAME_WE}.qm"
        BYPRODUCTS "${QM_BUILD_FILE}"
        VERBATIM
    )

    list(APPEND QM_FILES "${QM_FILE}")
endforeach()

add_custom_target(release_translations ALL DEPENDS ${QM_FILES})

# ----------------------------------------------------------------------------
# Platform-Specific Configuration
# ----------------------------------------------------------------------------
if(WIN32)
    set(logo_resource "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/logo.rc")
elseif(APPLE)
elseif(ANDROID)
    #include(Android)
elseif(LINUX)
    #include(Linux)
endif()

# ----------------------------------------------------------------------------
# Build Configuration
# ----------------------------------------------------------------------------
# Build Type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type config" FORCE)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()
if (BUILD_RELEASE_LITE)
    set(CONFIG_FILES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/res/configs/Release-Lite)
    set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release-Lite)
    message(STATUS "Build Release-Lite")
else()
    set(CONFIG_FILES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/res/configs/${CMAKE_BUILD_TYPE})
    set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# set output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH})
endif()
if (NOT LIBRARY_OUTPUT_DIRECTORY)
    set(LIBRARY_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/plugins)
endif()
if (NOT RUNTIME_OUTPUT_DIRECTORY)
    set(RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
endif()

# print build info
message(STATUS "Build type:                                       ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags, Debug configuration:                     ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "C flags, Release configuration:                   ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C flags, Release configuration with Debug info:   ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
message(STATUS "C flags, minimal Release configuration:           ${CMAKE_C_FLAGS_MINSIZEREL}")
message(STATUS "C++ flags, Debug configuration:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, Release configuration:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "C++ flags, minimal Release configuration:         ${CMAKE_CXX_FLAGS_MINSIZEREL}")

# ----------------------------------------------------------------------------
# submodules
# ----------------------------------------------------------------------------
add_subdirectory(3rd/rag-core)
add_subdirectory(3rd/libqt)

# ----------------------------------------------------------------------------
# Source Code
# ----------------------------------------------------------------------------
list(APPEND PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/res.qrc
)

add_subdirectory(src)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${CMAKE_PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rag-core/src/api.grpc.pb.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rag-core/src/api.pb.cc
        ${logo_resource}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${CMAKE_PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${CMAKE_PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui

    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network

    Qt${QT_VERSION_MAJOR}::Multimedia

    ${hj_LIBRARIES}

    whisper
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDE_DIRS}
    ${Qt_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rag-core
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libqt
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_dependencies(${CMAKE_PROJECT_NAME} release_translations)

install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${CMAKE_PROJECT_NAME})
endif()

# ----------------------------------------------------------------------------
# Config Files
# ----------------------------------------------------------------------------
file(GLOB RAG_QT_CONFIG_FILES CONFIGURE_DEPENDS
    "${CONFIG_FILES_PATH}/*.json"
)
if (RAG_QT_CONFIG_FILES)
    add_custom_target(rag_qt_copy_config_files ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/configs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RAG_QT_CONFIG_FILES}
            "${EXECUTABLE_OUTPUT_PATH}/configs"
        DEPENDS ${RAG_QT_CONFIG_FILES}
        COMMENT "Copying rag_qt config files to output dir ${EXECUTABLE_OUTPUT_PATH}"
    )
else()
    message(STATUS "No Rag-Qt config files to copy for ${PROJECT_NAME}")
endif()

# ----------------------------------------------------------------------------
# deploy Qt
# ----------------------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("install to ${EXECUTABLE_OUTPUT_PATH}")

    install(TARGETS ${PROJECT_NAME}
        DESTINATION "${EXECUTABLE_OUTPUT_PATH}"
    )

    include(3rd/libqt/cmakes/deployqt.cmake)
    if(WIN32)
        windeployqt(${PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH})
    elseif(APPLE)
        macdeployqt(${PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH})
    elseif(ANDROID)
    elseif(LINUX)
    endif()
endif()