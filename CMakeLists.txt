cmake_minimum_required(VERSION 3.20)

project(Cpp23CoodleProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main src/main.cpp)

add_library(systick src/systick_mac.c)
target_include_directories(systick PUBLIC src)
target_link_libraries(main PRIVATE systick)

