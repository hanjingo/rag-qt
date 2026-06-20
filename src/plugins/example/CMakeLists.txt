cmake_minimum_required(VERSION 3.16)

project(example VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

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

# set output path
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# print build info
message(STATUS "Project: example")
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
# Source Code
# ----------------------------------------------------------------------------

add_library(example MODULE
  example.h
  example.cpp
  example.ui

  ${CMAKE_CURRENT_SOURCE_DIR}/../Bus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Bus.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../PluginInterface.h
)

target_link_libraries(example PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(example PRIVATE example_LIBRARY)

# Export all symbols for Windows DLL
set_target_properties(example PROPERTIES 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    LIBRARY_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/plugins/example"
    RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/plugins/example"
)


# ----------------------------------------------------------------------------
# Resource Files
# ----------------------------------------------------------------------------
file(GLOB example_RESOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.jpg"
)
if (example_RESOURCE_FILES)
    add_custom_target(example_copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/plugins/example"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${example_RESOURCE_FILES}
            "${EXECUTABLE_OUTPUT_PATH}/plugins/example"
        DEPENDS ${example_RESOURCE_FILES}
        COMMENT "Copying example resource files to output dir"
    )
else()
    message(STATUS "No example resource files to copy for ${PROJECT_NAME}")
endif()