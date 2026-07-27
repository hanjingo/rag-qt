cmake_minimum_required(VERSION 3.16)

# ----------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------
project(rag-qt VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_RELEASE_LITE "Build release lite model" OFF)

# ----------------------------------------------------------------------------
# CMake Policies
# ----------------------------------------------------------------------------
if(POLICY CMP0168)
    cmake_policy(SET CMP0168 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0168 NEW)
endif()

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0141 NEW)
endif()

if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)
endif()

if(POLICY CMP0194)
    cmake_policy(SET CMP0194 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0194 NEW)
endif()

# ----------------------------------------------------------------------------
# Build Type Configuration
# ----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type config" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

if(BUILD_RELEASE_LITE)
    add_compile_definitions(LITE_APP)
    set(CONFIG_FILES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/res/configs/Release-Lite)
    set(OUTPUT_BASE_NAME "Release-Lite")
    message(STATUS "Build Release-Lite")
else()
    set(CONFIG_FILES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/res/configs/${CMAKE_BUILD_TYPE})
    set(OUTPUT_BASE_NAME ${CMAKE_BUILD_TYPE})
endif()

# ----------------------------------------------------------------------------
# Platform Detection and Output Directory Configuration
# ----------------------------------------------------------------------------
set(OUTPUT_DIR_BASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")

if(WIN32)
    # Windows: Standard layout - all files in same directory
    # Structure: bin/Release/rag-qt.exe, bin/Release/plugins/chatbox.dll, bin/Release/configs/, bin/Release/res/
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    
    # Multi-configuration support
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CFG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
            string(TOUPPER ${CFG} CFG_UPPER)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}")
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}/plugins")
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}")
        endforeach()
    endif()
    
    set(EXECUTABLE_OUTPUT_PATH "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
    set(RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(RESOURCE_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/res")
    set(CONFIG_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/configs")
    set(PLUGIN_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
    
elseif(APPLE)
    # macOS: Bundle structure
    # Structure: bin/Release/rag-qt.app/Contents/MacOS/rag-qt, PlugIns/, Resources/
    set(APP_BUNDLE_NAME "${CMAKE_PROJECT_NAME}.app")
    set(APP_BUNDLE_PATH "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/${APP_BUNDLE_NAME}")
    set(APP_CONTENTS_PATH "${APP_BUNDLE_PATH}/Contents")
    set(APP_MACOS_PATH "${APP_CONTENTS_PATH}/MacOS")
    set(APP_RESOURCES_PATH "${APP_CONTENTS_PATH}/Resources")
    set(APP_PLUGINS_PATH "${APP_CONTENTS_PATH}/PlugIns")
    
    # Executable goes inside .app bundle
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${APP_MACOS_PATH}")
    # Plugins go inside .app/Contents/PlugIns/
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${APP_PLUGINS_PATH}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CFG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
            string(TOUPPER ${CFG} CFG_UPPER)
            set(CFG_APP_BUNDLE_PATH "${OUTPUT_DIR_BASE}/${CFG}/${CMAKE_PROJECT_NAME}.app")
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CFG_APP_BUNDLE_PATH}/Contents/MacOS")
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CFG_APP_BUNDLE_PATH}/Contents/PlugIns")
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}")
        endforeach()
    endif()
    
    set(EXECUTABLE_OUTPUT_PATH "${APP_MACOS_PATH}")
    set(LIBRARY_OUTPUT_DIRECTORY "${APP_PLUGINS_PATH}")
    set(RUNTIME_OUTPUT_DIRECTORY "${APP_MACOS_PATH}")
    set(RESOURCE_OUTPUT_DIRECTORY "${APP_RESOURCES_PATH}/res")
    set(CONFIG_OUTPUT_DIRECTORY "${APP_RESOURCES_PATH}/configs")
    set(PLUGIN_OUTPUT_DIRECTORY "${APP_PLUGINS_PATH}")
    
elseif(UNIX)
    # Linux: Standard FHS-like layout with prefix
    # Structure: bin/Release/rag-qt, bin/Release/plugins/chatbox.so, share/rag-qt/configs/, share/rag-qt/res/
    # We keep it simple: all in bin/Release/ with subdirectories
    
    # For Linux, we use a prefix-based structure similar to /usr/local
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(CFG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
            string(TOUPPER ${CFG} CFG_UPPER)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}")
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}/plugins")
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${OUTPUT_DIR_BASE}/${CFG}")
        endforeach()
    endif()
    
    set(EXECUTABLE_OUTPUT_PATH "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
    set(RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
    set(RESOURCE_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/share/rag-qt/res")
    set(CONFIG_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/share/rag-qt/configs")
    set(PLUGIN_OUTPUT_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}/plugins")
endif()

# Export variables for submodules
set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}" CACHE STRING "Executable output path" FORCE)
set(LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}" CACHE STRING "Library output directory" FORCE)
set(RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}" CACHE STRING "Runtime output directory" FORCE)
set(RESOURCE_OUTPUT_DIRECTORY "${RESOURCE_OUTPUT_DIRECTORY}" CACHE STRING "Resource output directory" FORCE)
set(CONFIG_OUTPUT_DIRECTORY "${CONFIG_OUTPUT_DIRECTORY}" CACHE STRING "Config output directory" FORCE)
set(PLUGIN_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIRECTORY}" CACHE STRING "Plugin output directory" FORCE)

message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Output directory: ${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
if(APPLE)
    message(STATUS "App bundle: ${APP_BUNDLE_PATH}")
    message(STATUS "  MacOS: ${APP_MACOS_PATH}")
    message(STATUS "  Resources: ${APP_RESOURCES_PATH}")
    message(STATUS "  PlugIns: ${APP_PLUGINS_PATH}")
else()
    message(STATUS "Plugin output directory: ${PLUGIN_OUTPUT_DIRECTORY}")
    message(STATUS "Resource output directory: ${RESOURCE_OUTPUT_DIRECTORY}")
    message(STATUS "Config output directory: ${CONFIG_OUTPUT_DIRECTORY}")
endif()

# ----------------------------------------------------------------------------
# Qt
# ----------------------------------------------------------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Multimedia)

# Qt private includes (for zip, etc.)
set(Qt_PRIVATE_INCLUDES 
    ${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS}
    ${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS}
)

# Get Qt bin directory for deployment
get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
if(QT_QMAKE_EXECUTABLE)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
else()
    find_program(QT_QMAKE_EXECUTABLE NAMES qmake-qt6 qmake6 qmake)
    if(QT_QMAKE_EXECUTABLE)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    endif()
endif()

# ----------------------------------------------------------------------------
# 3rd Party Libraries
# ----------------------------------------------------------------------------
set(hj_LIBRARIES "")
set(hj_INCLUDE_DIRS "")

# Boost
find_package(Boost QUIET COMPONENTS system filesystem)
if(Boost_FOUND)
    list(APPEND hj_LIBRARIES ${Boost_LIBRARIES})
    list(APPEND hj_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
endif()

# fmt
find_package(fmt CONFIG QUIET)
if(fmt_FOUND)
    list(APPEND hj_LIBRARIES fmt::fmt)
endif()

# nlohmann_json
find_package(nlohmann_json CONFIG QUIET)
if(nlohmann_json_FOUND)
    list(APPEND hj_LIBRARIES nlohmann_json::nlohmann_json)
endif()

# GTest
find_package(GTest CONFIG QUIET)
if(GTest_FOUND)
    list(APPEND hj_LIBRARIES GTest::gtest GTest::gtest_main)
endif()

# benchmark
find_package(benchmark CONFIG QUIET)
if(benchmark_FOUND)
    list(APPEND hj_LIBRARIES benchmark::benchmark)
endif()

# breakpad
find_package(unofficial-breakpad CONFIG QUIET)
if(unofficial-breakpad_FOUND)
    list(APPEND hj_LIBRARIES 
        unofficial::breakpad::libbreakpad 
        unofficial::breakpad::libbreakpad_client
    )
endif()

# opentelemetry
find_package(opentelemetry-cpp CONFIG QUIET)
if(opentelemetry-cpp_FOUND)
    list(APPEND hj_LIBRARIES ${OPENTELEMETRY_CPP_LIBRARIES})
endif()

# spdlog
find_package(spdlog CONFIG QUIET)
if(spdlog_FOUND)
    list(APPEND hj_LIBRARIES spdlog::spdlog)
endif()

# Boost program_options
find_package(Boost QUIET COMPONENTS program_options)
if(Boost_PROGRAM_OPTIONS_FOUND)
    list(APPEND hj_LIBRARIES Boost::program_options)
endif()

# gRPC
find_package(gRPC CONFIG QUIET)
if(gRPC_FOUND)
    list(APPEND hj_LIBRARIES gRPC::grpc gRPC::grpc++)
    if(WIN32)
        list(APPEND hj_LIBRARIES gRPC::grpc++_reflection)
    endif()
endif()

# SQLite3
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    list(APPEND hj_LIBRARIES ${SQLite3_LIBRARIES})
    list(APPEND hj_INCLUDE_DIRS ${SQLite3_INCLUDE_DIRS})
endif()

# TBB
find_package(TBB CONFIG QUIET)
if(TBB_FOUND)
    list(APPEND hj_LIBRARIES TBB::tbb)
endif()

# jwt-cpp
find_package(jwt-cpp CONFIG QUIET)
if(jwt-cpp_FOUND)
    list(APPEND hj_LIBRARIES jwt-cpp::jwt-cpp)
endif()

# Llama
find_package(Llama CONFIG QUIET)
if(Llama_FOUND)
    list(APPEND hj_LIBRARIES llama)
endif()

# OpenMP
find_package(OpenMP QUIET COMPONENTS CXX)
if(OpenMP_CXX_FOUND)
    list(APPEND hj_LIBRARIES OpenMP::OpenMP_CXX)
endif()

# Faiss
find_package(faiss CONFIG QUIET)
if(faiss_FOUND)
    list(APPEND hj_LIBRARIES faiss)
endif()

# Whisper
find_package(whisper CONFIG QUIET)
if(whisper_FOUND)
    list(APPEND hj_LIBRARIES whisper)
endif()

# ----------------------------------------------------------------------------
# Include Directories
# ----------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/high-jump)

# ----------------------------------------------------------------------------
# Translations
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
    
    if(APPLE)
        set(QM_FILE "${RESOURCE_OUTPUT_DIRECTORY}/languages/${TS_NAME_WE}.qm")
    else()
        set(QM_FILE "${RESOURCE_OUTPUT_DIRECTORY}/languages/${TS_NAME_WE}.qm")
    endif()

    add_custom_command(
        OUTPUT "${QM_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/res/languages"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${RESOURCE_OUTPUT_DIRECTORY}/languages"
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
else()
    set(logo_resource "")
endif()

# ----------------------------------------------------------------------------
# Submodules
# ----------------------------------------------------------------------------
add_subdirectory(3rd/libqt)
if(NOT BUILD_RELEASE_LITE)
    add_subdirectory(3rd/rag-core)
endif()

# ----------------------------------------------------------------------------
# Source Code
# ----------------------------------------------------------------------------
list(APPEND PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/res.qrc
)

add_subdirectory(src)

# ----------------------------------------------------------------------------
# Executable / Library
# ----------------------------------------------------------------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${CMAKE_PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${logo_resource}
    )
else()
    if(ANDROID)
        add_library(${CMAKE_PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(${CMAKE_PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Add protobuf generated files if they exist
set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rag-core/src")
if(EXISTS "${PROTO_GEN_DIR}/api.grpc.pb.cc" AND EXISTS "${PROTO_GEN_DIR}/api.pb.cc")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        "${PROTO_GEN_DIR}/api.grpc.pb.cc"
        "${PROTO_GEN_DIR}/api.pb.cc"
    )
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${PROTO_GEN_DIR}")
endif()

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${hj_LIBRARIES}
)

# Include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDE_DIRS}
    ${Qt_PRIVATE_INCLUDES}
    ${hj_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rag-core
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libqt
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/high-jump
)

# Set target properties
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.rag-qt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
endif()

add_dependencies(${CMAKE_PROJECT_NAME} release_translations)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    BUNDLE DESTINATION .
)

# ----------------------------------------------------------------------------
# Copy Config Files
# ----------------------------------------------------------------------------
file(GLOB RAG_QT_CONFIG_FILES CONFIGURE_DEPENDS
    "${CONFIG_FILES_PATH}/*.json"
)

if(RAG_QT_CONFIG_FILES)
    add_custom_target(copy_config_files ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CONFIG_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RAG_QT_CONFIG_FILES}
            "${CONFIG_OUTPUT_DIRECTORY}"
        DEPENDS ${RAG_QT_CONFIG_FILES}
        COMMENT "Copying config files to ${CONFIG_OUTPUT_DIRECTORY}"
    )
    add_dependencies(${CMAKE_PROJECT_NAME} copy_config_files)
else()
    message(STATUS "No config files to copy")
endif()

# ----------------------------------------------------------------------------
# Copy Resource Files (translations, etc.)
# ----------------------------------------------------------------------------
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_OUTPUT_DIRECTORY}/languages"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QM_FILES}
        "${RESOURCE_OUTPUT_DIRECTORY}/languages"
    DEPENDS ${QM_FILES}
    COMMENT "Copying resource files to ${RESOURCE_OUTPUT_DIRECTORY}/languages"
)
add_dependencies(${CMAKE_PROJECT_NAME} copy_resources)

# ----------------------------------------------------------------------------
# Deploy Qt and Dependencies
# ----------------------------------------------------------------------------
include(deploy.cmake OPTIONAL)

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT ANDROID)
    if(WIN32)
        # Find windeployqt
        find_program(WINDEPLOYQT_EXECUTABLE 
            NAMES windeployqt6 windeployqt-qt6 windeployqt
            HINTS ${QT_BIN_DIR}
            PATHS "${QT_BIN_DIR}"
        )
        
        if(WINDEPLOYQT_EXECUTABLE)
            execute_process(
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --help
                OUTPUT_VARIABLE WINDEPLOYQT_HELP
                ERROR_QUIET
            )
            
            set(WINDEPLOYQT_ARGS --no-compiler-runtime --no-system-d3d-compiler --verbose 0)
            
            if(WINDEPLOYQT_HELP MATCHES "--no-angle")
                list(APPEND WINDEPLOYQT_ARGS --no-angle)
            endif()
            
            if(WINDEPLOYQT_HELP MATCHES "--no-opengl-sw")
                list(APPEND WINDEPLOYQT_ARGS --no-opengl-sw)
            endif()
            
            # For Windows, deploy Qt libraries to the executable directory
            set(WIN_DEPLOY_DIR "${EXECUTABLE_OUTPUT_PATH}")
            
            add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${WIN_DEPLOY_DIR}"
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    ${WINDEPLOYQT_ARGS}
                    "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
                COMMENT "Deploying Qt libraries to ${WIN_DEPLOY_DIR}"
                WORKING_DIRECTORY "${WIN_DEPLOY_DIR}"
            )
            
            # Use deploy.cmake to copy additional DLLs
            if(COMMAND win_deploy)
                win_deploy(${CMAKE_PROJECT_NAME} ${hj_LIBRARIES})
            endif()
        else()
            message(WARNING "windeployqt not found, skipping Qt deployment")
        endif()
        
    elseif(APPLE)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt)
        if(MACDEPLOYQT_EXECUTABLE)
            # For macOS, deploy Qt frameworks into the .app bundle
            add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND "${MACDEPLOYQT_EXECUTABLE}"
                    "${APP_BUNDLE_PATH}"
                    -always-overwrite
                    -verbose=0
                    -executable="${APP_MACOS_PATH}/${CMAKE_PROJECT_NAME}"
                COMMENT "Deploying Qt frameworks to ${APP_BUNDLE_PATH}"
                WORKING_DIRECTORY "${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}"
            )
            
            # Use deploy.cmake to copy additional dylibs
            if(COMMAND mac_deploy)
                mac_deploy(${CMAKE_PROJECT_NAME} ${hj_LIBRARIES})
            endif()
        else()
            message(WARNING "macdeployqt not found, skipping Qt deployment")
        endif()
        
    elseif(LINUX)
        # For Linux, we rely on system package manager or LD_LIBRARY_PATH
        # Optionally use linuxdeployqt for AppImage
        find_program(LINUXDEPLOYQT_EXECUTABLE linuxdeployqt)
        if(LINUXDEPLOYQT_EXECUTABLE AND EXISTS "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}")
            add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                COMMAND "${LINUXDEPLOYQT_EXECUTABLE}"
                    "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}"
                    -bundle-non-qt-libs
                    -verbose=0
                COMMENT "Deploying Qt libraries using linuxdeployqt"
                WORKING_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
            )
        else()
            message(STATUS "linuxdeployqt not found, skipping Qt deployment")
        endif()
        
        # Use deploy.cmake to copy additional so files
        if(COMMAND linux_deploy)
            linux_deploy(${CMAKE_PROJECT_NAME} ${hj_LIBRARIES})
        endif()
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${CMAKE_PROJECT_NAME})
endif()

# ----------------------------------------------------------------------------
# Print Build Information
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "==================== Build Configuration ====================")
message(STATUS "Platform:              ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type:            ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:          ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version:            ${QT_VERSION_MAJOR}")
message(STATUS "Output directory:      ${OUTPUT_DIR_BASE}/${CMAKE_BUILD_TYPE}")
if(APPLE)
    message(STATUS "App bundle:            ${APP_BUNDLE_PATH}")
    message(STATUS "  MacOS:               ${APP_MACOS_PATH}")
    message(STATUS "  Resources:           ${APP_RESOURCES_PATH}")
    message(STATUS "  PlugIns:             ${APP_PLUGINS_PATH}")
else()
    message(STATUS "Plugins:               ${PLUGIN_OUTPUT_DIRECTORY}")
    message(STATUS "Resources:             ${RESOURCE_OUTPUT_DIRECTORY}")
    message(STATUS "Configs:               ${CONFIG_OUTPUT_DIRECTORY}")
endif()
message(STATUS "=============================================================")
message(STATUS "")