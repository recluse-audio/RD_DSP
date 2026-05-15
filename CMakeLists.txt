cmake_minimum_required(VERSION 3.24.1)

set(PROJECT_NAME "RD_DSP")

# Read version
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" CURRENT_VERSION)
string(STRIP "${CURRENT_VERSION}" CURRENT_VERSION)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION} LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Options — default ON when top-level, OFF when embedded as subdirectory
if(PROJECT_IS_TOP_LEVEL)
    set(_rd_dsp_default ON)
else()
    set(_rd_dsp_default OFF)
endif()
option(RD_DSP_BUILD_TESTS      "Build RD_DSP Catch2 unit tests"   ${_rd_dsp_default})
option(RD_DSP_BUILD_STANDALONE "Build RD_DSP standalone CLI demo" ${_rd_dsp_default})

# /FS prevents PDB contention on parallel MSVC builds
if(MSVC)
    add_compile_options(/FS)
endif()

# macOS: universal binary on CI Release
if((DEFINED ENV{CI}) AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")

# ---------------------------------------------------------------------------
# Core library: RD_DSP (header-only friendly, zero deps)
# ---------------------------------------------------------------------------
include(CMAKE/SOURCES.cmake)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SOURCE
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Allow empty source list during early scaffolding
get_target_property(_rd_dsp_srcs ${PROJECT_NAME} SOURCES)
if(NOT _rd_dsp_srcs)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
endif()

# ---------------------------------------------------------------------------
# Standalone CLI demo
# ---------------------------------------------------------------------------
if(RD_DSP_BUILD_STANDALONE)
    add_executable(RD_DSP_Standalone STANDALONE/main.cpp)
    target_link_libraries(RD_DSP_Standalone PRIVATE ${PROJECT_NAME})
endif()

# ---------------------------------------------------------------------------
# Tests (Catch2 v3 via FetchContent)
# ---------------------------------------------------------------------------
if(RD_DSP_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)

    include(CMAKE/TESTS.cmake)
    add_executable(Tests ${TEST_SOURCES})
    target_link_libraries(Tests PRIVATE ${PROJECT_NAME} Catch2::Catch2)
    target_include_directories(Tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/TESTS
    )
    target_compile_definitions(Tests PRIVATE
        RD_DSP_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/TESTS"
    )

    enable_testing()
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(Tests)
endif()
