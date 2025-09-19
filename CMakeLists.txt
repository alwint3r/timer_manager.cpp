cmake_minimum_required(VERSION 3.20)

if (ESP_PLATFORM)
	idf_component_register(INCLUDE_DIRS "include")
else()
	project(timer_manager LANGUAGES C CXX)

	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_C_STANDARD 17)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	
	if(APPLE)
    add_subdirectory(example_mac)
	else()
    message(STATUS "Skipping macOS example target because APPLE is not defined.")
	endif()
endif()

