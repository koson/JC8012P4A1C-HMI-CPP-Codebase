cmake_minimum_required(VERSION 3.16)
project(LabBuddy_PC_Simulator C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Compile definitions for LVGL PC simulation
add_compile_definitions(LV_USE_SDL=1)
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE=1)
add_compile_definitions(SDL_MAIN_HANDLED=1)

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON
    ${PROJECT_ROOT_DIR}/main
    ${PROJECT_ROOT_DIR}/main/include
    ${PROJECT_ROOT_DIR}/managed_components
    ${PROJECT_ROOT_DIR}/managed_components/lvgl__lvgl
    ${PROJECT_ROOT_DIR}/managed_components/lvgl__lvgl/src
    ${PROJECT_ROOT_DIR}/components/json_renderer/include
    ${PROJECT_ROOT_DIR}/components/svg_renderer/include
    ${PROJECT_ROOT_DIR}/components/lvgl_cpp_v9/include
    ${PROJECT_ROOT_DIR}/components/vpi_document/include
    ${PROJECT_ROOT_DIR}/components/font_thai/include
    ${SDL2_INCLUDE_DIRS}
)

# Add LVGL library target from managed_components
add_subdirectory(${PROJECT_ROOT_DIR}/managed_components/lvgl__lvgl ${CMAKE_BINARY_DIR}/lvgl)

# Source files from components & main
file(GLOB_RECURSE JSON_RENDERER_SOURCES "${PROJECT_ROOT_DIR}/components/json_renderer/src/*.cpp" "${PROJECT_ROOT_DIR}/components/json_renderer/src/*.c")
file(GLOB_RECURSE SVG_RENDERER_SOURCES "${PROJECT_ROOT_DIR}/components/svg_renderer/src/*.cpp" "${PROJECT_ROOT_DIR}/components/svg_renderer/src/*.c")
file(GLOB_RECURSE LVGL_CPP_SOURCES "${PROJECT_ROOT_DIR}/components/lvgl_cpp_v9/src/*.cpp" "${PROJECT_ROOT_DIR}/components/lvgl_cpp_v9/src/*.c")
file(GLOB_RECURSE FONT_THAI_SOURCES "${PROJECT_ROOT_DIR}/components/font_thai/src/*.c")

set(SIMULATOR_SOURCES
    main.cpp
    pc_stubs.cpp
    hal/hal.c
    hal/mouse_cursor_icon.c
    cJSON/cJSON.c
    ${PROJECT_ROOT_DIR}/main/img_pressure_needle.c
    ${PROJECT_ROOT_DIR}/main/cpp_version/LessonPlayer.cpp
    ${JSON_RENDERER_SOURCES}
    ${SVG_RENDERER_SOURCES}
    ${LVGL_CPP_SOURCES}
    ${FONT_THAI_SOURCES}
)

add_executable(LabBuddy_Simulator ${SIMULATOR_SOURCES})

target_link_libraries(LabBuddy_Simulator PRIVATE
    lvgl
    -lSDL2
)

# Locate SDL2.dll dynamically across machines
find_file(SDL2_DLL_PATH SDL2.dll PATHS
    "C:/SDL2/lib/x64"
    "C:/msys64/ucrt64/bin"
    "C:/msys64/mingw64/bin"
    "$ENV{SDL2_DIR}/lib/x64"
    "$ENV{SDL2_DIR}/bin"
    "$ENV{SDL2_DIR}"
)

if(NOT SDL2_DLL_PATH)
    set(SDL2_DLL_PATH "C:/SDL2/lib/x64/SDL2.dll")
endif()

# Copy SDL2.dll & worksheets directory to binary folder automatically post-build
add_custom_command(TARGET LabBuddy_Simulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DLL_PATH}"
    $<TARGET_FILE_DIR:LabBuddy_Simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/worksheets"
    $<TARGET_FILE_DIR:LabBuddy_Simulator>/worksheets
)
