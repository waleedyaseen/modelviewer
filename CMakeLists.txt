cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(3rdParty/glad)
add_subdirectory(src)

project(modelviewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(IMGUI)
include(GLFW)
include(json)
include(glm)

# ============================================
# - Setup GIT_COMMIT_HASH macro
# ============================================
find_package(Git QUIET)
if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if (GIT_COMMIT_HASH)
        add_compile_definitions(GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
    else ()
        add_compile_definitions(GIT_COMMIT_HASH="unknown")
    endif ()
else ()
    add_compile_definitions(GIT_COMMIT_HASH="unknown")
endif ()

# ================================================
# - Setup common macro definitions for the project
# ================================================
include(TestBigEndian)
test_big_endian(IMP_BIG_ENDIAN)
if (IMP_BIG_ENDIAN)
    add_compile_definitions(IMP_ENDIANNESS_BIG)
else ()
    add_compile_definitions(IMP_ENDIANNESS_LITTLE)
endif ()
if (WIN32)
    add_compile_definitions(IMP_PLATFORM_WINDOWS)
elseif (UNIX)
    add_compile_definitions(IMP_PLATFORM_LINUX)
elseif (APPLE)
    add_compile_definitions(IMP_PLATFORM_APPLE)
endif ()
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_CUSTOM=1)

# =======================================================
# - Setup Dear ImGui source files to be part of the build
# =======================================================
set(IMGUI_SRC_FILES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
)

# ======================================
# - Setup platform specific source files
# ======================================
if (WIN32)
    set(PLATFORM_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/app.rc
    )
else ()
    set(PLATFORM_FILES
    )
endif ()

# ======================================================
# - Setup executable output and link/include directories
# ======================================================
if (WIN32)
    add_executable(modelviewer
            WIN32
            ${ROOT_SRC_FILES}
            ${IMGUI_SRC_FILES}
            ${PLATFORM_FILES}
    )
    target_link_libraries(modelviewer PRIVATE user32)
else ()
    add_executable(modelviewer
            WIN32
            ${ROOT_SRC_FILES}
            ${IMGUI_SRC_FILES}
            ${PLATFORM_FILES}
    )
endif ()
target_link_libraries(modelviewer PRIVATE glad glfw nlohmann_json::nlohmann_json glm::glm)
target_include_directories(modelviewer PUBLIC
        ${imgui_SOURCE_DIR}
        ${glfw_SOURCE_DIR}/include
        ${json_SOURCE_DIR}/include
        ${GLM_INCLUDE_DIRS}
        3rdParty/glm
)

# =========================================================
# - Copy anything in fonts directory to the build directory
# =========================================================
file(GLOB_RECURSE FONTS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*")
foreach (FONT_FILE ${FONTS_FILES})
    get_filename_component(FONT_FILE_NAME ${FONT_FILE} NAME)
    add_custom_command(TARGET modelviewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FONT_FILE}"
            "${CMAKE_CURRENT_BINARY_DIR}/fonts/${FONT_FILE_NAME}"
    )
endforeach ()


# =============================================================
# - Copy default imgui.ini configuration to the build directory
# =============================================================
add_custom_command(TARGET modelviewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
        "${CMAKE_CURRENT_BINARY_DIR}/imgui.ini"
)