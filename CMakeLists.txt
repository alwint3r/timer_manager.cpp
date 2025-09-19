cmake_minimum_required(VERSION 3.20)

project(timer_manager LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(timer_manager INTERFACE)
target_include_directories(timer_manager
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_features(timer_manager INTERFACE cxx_std_23)

if(APPLE)
    add_subdirectory(example_mac)
else()
    message(STATUS "Skipping macOS example target because APPLE is not defined.")
endif()
