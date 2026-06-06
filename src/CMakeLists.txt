# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(util)
# add_subdirectory(login_page)
add_subdirectory(framework)
add_subdirectory(home_page)
add_subdirectory(audio_page)
add_subdirectory(chat_page)
add_subdirectory(image_page)
add_subdirectory(setting_page)
add_subdirectory(text_page)

list(APPEND PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(PROJECT_SOURCES "${PROJECT_SOURCES}" PARENT_SCOPE)

set(PROJECT_INCLUDE_DIRS "${PROJECT_INCLUDE_DIRS}" PARENT_SCOPE)
