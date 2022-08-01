cmake_minimum_required(VERSION 3.12)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        # Update submodules as needed
        option(GIT_SUBMODULE "Check submodules during build" ON)
                if(GIT_SUBMODULE)
                        message(STATUS "Submodule update")
                        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
                if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
                endif()
        endif()
endif()

# Pull in SDK (must be before project)
set(PICO_SDK_PATH pico-sdk)
include(pico-sdk/external/pico_sdk_import.cmake)

project(pico_p1p2_decoder C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Wno-narrowing
        -fno-exceptions
        -fno-non-call-exceptions
        -funroll-loops
        -g -O2
        )

# This line is last
add_subdirectory(src)
