cmake_minimum_required(VERSION 3.24)
project(rift_runner_signal_zero VERSION 0.26.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C99 with compiler extensions enabled for both targets. Emscripten's
# EM_ASM macros require GNU C mode, and raylib/GLFW desktop dependencies can
# rely on POSIX/GNU feature definitions. The language standard remains C99.
set(CMAKE_C_EXTENSIONS ON)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "raylib target platform" FORCE)
endif()

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build raylib examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Do not build raylib games" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Do not build raylib tests" FORCE)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)

if(EMSCRIPTEN)
    # Override any strict standard set by raylib's own CMake configuration.
    set_property(TARGET raylib PROPERTY C_EXTENSIONS ON)
    target_compile_options(raylib PRIVATE $<$<COMPILE_LANGUAGE:C>:-std=gnu99>)
endif()

add_executable(rift_runner
    main.c
    game.c
    helpers.c
    colors.c
    background.c
    player.c
    enemies.c
    bullets.c
    pickups.c
    effects.c
    world.c
    audio.c
    leaderboard.c
    ui.c
)
target_link_libraries(rift_runner PRIVATE raylib)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rift_runner PRIVATE -Wall -Wextra -Wpedantic -Werror=implicit-function-declaration)
endif()
if(EMSCRIPTEN)
    set_property(TARGET rift_runner PROPERTY C_EXTENSIONS ON)
    target_compile_options(rift_runner PRIVATE -std=gnu99)
    target_link_options(rift_runner PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sFORCE_FILESYSTEM=1"
        "-sASSERTIONS=1"
        "-sGL_ENABLE_GET_PROC_ADDRESS=1"
        "--preload-file" "${CMAKE_SOURCE_DIR}/assets@/assets"
        "--pre-js" "${CMAKE_SOURCE_DIR}/web/idbfs-pre.js"
        "-lidbfs.js"
    )
    set_target_properties(rift_runner PROPERTIES SUFFIX ".html")
else()
    if(UNIX AND NOT APPLE)
        target_link_libraries(rift_runner PRIVATE m)
    endif()
    add_custom_command(TARGET rift_runner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:rift_runner>/assets"
        VERBATIM
    )
endif()

set_target_properties(rift_runner PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config_name DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(rift_runner PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_name} "${CMAKE_BINARY_DIR}/bin")
endforeach()
