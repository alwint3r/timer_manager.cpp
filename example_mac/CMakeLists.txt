set(EXAMPLE_MAC_SOURCES
    main.cpp
    systick_mac.c
)

add_executable(example_mac ${EXAMPLE_MAC_SOURCES})
target_compile_features(example_mac PRIVATE cxx_std_23)
target_compile_options(example_mac PRIVATE $<$<COMPILE_LANGUAGE:C>:-fblocks>)
target_include_directories(example_mac PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
