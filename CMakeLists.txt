cmake_minimum_required(VERSION 3.24)
project(rift_runner_signal_zero VERSION 0.15.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RR_BUILD_TESTS "Build RIFT RUNNER regression tests" ON)
option(RR_TESTS_ONLY "Configure only offline/headless tests (does not fetch raylib)" OFF)
option(RR_USE_SYSTEM_RAYLIB "Use an installed raylib CMake package instead of fetching raylib" OFF)
option(RR_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)
set(RR_RAYLIB_SOURCE_DIR "" CACHE PATH "Optional local raylib source tree; skips the download when set")

if(RR_ENABLE_SANITIZERS AND NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    message(FATAL_ERROR "RR_ENABLE_SANITIZERS requires GCC or Clang; disable it for this compiler.")
endif()

function(rr_enable_sanitizers target_name)
    if(RR_ENABLE_SANITIZERS)
        target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

if(RR_USE_SYSTEM_RAYLIB AND RR_RAYLIB_SOURCE_DIR)
    message(FATAL_ERROR "Choose only one: RR_USE_SYSTEM_RAYLIB=ON or RR_RAYLIB_SOURCE_DIR=<path>")
endif()

if(NOT RR_TESTS_ONLY)
    if(RR_USE_SYSTEM_RAYLIB)
        # Prefer a package-provided CMake target; some distro packages expose
        # only raylib.pc, so fall back to pkg-config before reporting failure.
        find_package(raylib CONFIG QUIET)
        set(RR_RAYLIB_TARGET "")
        if(TARGET raylib)
            set(RR_RAYLIB_TARGET raylib)
        elseif(TARGET raylib::raylib)
            set(RR_RAYLIB_TARGET raylib::raylib)
        elseif(TARGET raylib_static)
            set(RR_RAYLIB_TARGET raylib_static)
        elseif(TARGET raylib_shared)
            set(RR_RAYLIB_TARGET raylib_shared)
        endif()

        if(NOT RR_RAYLIB_TARGET)
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(RR_SYSTEM_RAYLIB QUIET IMPORTED_TARGET raylib)
                if(TARGET PkgConfig::RR_SYSTEM_RAYLIB)
                    set(RR_RAYLIB_TARGET PkgConfig::RR_SYSTEM_RAYLIB)
                endif()
            endif()
        endif()

        if(NOT RR_RAYLIB_TARGET)
            message(FATAL_ERROR
                "RR_USE_SYSTEM_RAYLIB=ON, but no usable raylib CMake target or pkg-config module was found. "
                "Install raylib with development headers and either a CMake package or raylib.pc, "
                "or use -DRR_RAYLIB_SOURCE_DIR=/path/to/raylib.")
        endif()
        message(STATUS "Using installed raylib target: ${RR_RAYLIB_TARGET}")
    else()
        include(FetchContent)
        set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build raylib examples" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "Do not build raylib games" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "Do not build raylib tests" FORCE)
        if(RR_RAYLIB_SOURCE_DIR)
            if(NOT EXISTS "${RR_RAYLIB_SOURCE_DIR}/CMakeLists.txt")
                message(FATAL_ERROR "RR_RAYLIB_SOURCE_DIR must point to the root of a raylib source tree containing CMakeLists.txt")
            endif()
            set(FETCHCONTENT_SOURCE_DIR_RAYLIB "${RR_RAYLIB_SOURCE_DIR}" CACHE PATH "Local raylib source tree" FORCE)
        endif()
        FetchContent_Declare(
            raylib
            URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(raylib)
        set(RR_RAYLIB_TARGET raylib)
        if(RR_RAYLIB_SOURCE_DIR)
            message(STATUS "Using local raylib source tree: ${RR_RAYLIB_SOURCE_DIR}")
        else()
            message(STATUS "Using fetched raylib 6.0")
        endif()
    endif()

    add_executable(rift_runner
        src/main.c
        src/game.c
        src/render.c
        src/audio.c
        src/persistence.c
    )
    target_include_directories(rift_runner PRIVATE src)
    target_link_libraries(rift_runner PRIVATE ${RR_RAYLIB_TARGET})
    rr_enable_sanitizers(rift_runner)

    if(MSVC)
        target_compile_options(rift_runner PRIVATE /W4)
    else()
        target_compile_options(rift_runner PRIVATE -Wall -Wextra -Wpedantic)
        if(UNIX AND NOT APPLE)
            target_link_libraries(rift_runner PRIVATE m)
        endif()
    endif()

    if(APPLE)
        target_link_libraries(rift_runner PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    endif()

    # Keep the executable in the same directory with single- and multi-config
    # generators (e.g. Make/Ninja and Visual Studio/Xcode).
    set_target_properties(rift_runner PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    foreach(config_name DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(rift_runner PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${config_name} "${CMAKE_BINARY_DIR}/bin"
        )
    endforeach()

    add_custom_command(TARGET rift_runner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:rift_runner>/assets"
        VERBATIM
    )
else()
    message(STATUS "RR_TESTS_ONLY=ON: skipping the game executable and raylib dependency")
endif()

if(RR_BUILD_TESTS)
    enable_testing()

    add_executable(rift_runner_profile_tests
        tests/test_profile.c
        src/persistence.c
    )
    target_include_directories(rift_runner_profile_tests PRIVATE tests/raylib_stub src)
    rr_enable_sanitizers(rift_runner_profile_tests)

    add_executable(rift_runner_logic_tests
        tests/test_game_logic.c
        tests/raylib_stub/raylib_stub.c
        src/game.c
        src/persistence.c
    )
    target_include_directories(rift_runner_logic_tests PRIVATE tests/raylib_stub src)
    rr_enable_sanitizers(rift_runner_logic_tests)

    if(MSVC)
        target_compile_options(rift_runner_profile_tests PRIVATE /W4)
        target_compile_options(rift_runner_logic_tests PRIVATE /W4)
    else()
        target_compile_options(rift_runner_profile_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(rift_runner_logic_tests PRIVATE -Wall -Wextra -Wpedantic)
        if(UNIX AND NOT APPLE)
            target_link_libraries(rift_runner_profile_tests PRIVATE m)
            target_link_libraries(rift_runner_logic_tests PRIVATE m)
        endif()
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-profile-work")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-logic-work")
    add_test(NAME profile_persistence COMMAND rift_runner_profile_tests)
    add_test(NAME gameplay_core_surge_logic COMMAND rift_runner_logic_tests)
    set_tests_properties(profile_persistence PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-profile-work")
    set_tests_properties(gameplay_core_surge_logic PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-logic-work")
endif()
