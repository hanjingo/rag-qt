# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(splash_page)
add_subdirectory(login_page)
add_subdirectory(framework)
add_subdirectory(main_page)
add_subdirectory(setting_page)

add_subdirectory(plugins)

list(APPEND PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(PROJECT_SOURCES "${PROJECT_SOURCES}" PARENT_SCOPE)

set(PROJECT_INCLUDE_DIRS "${PROJECT_INCLUDE_DIRS}" PARENT_SCOPE)
