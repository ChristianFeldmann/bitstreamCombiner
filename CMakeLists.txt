cmake_minimum_required(VERSION 3.14)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

project(bitstreamCombiner)

FILE(GLOB_RECURSE SOURCE_FILES src/*.cpp)
FILE(GLOB_RECURSE HEADER_FILES src/*.h)

add_executable(bitstreamCombiner ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(bitstreamCombiner PRIVATE ${CMAKE_SOURCE_DIR}/src)

set_target_properties(bitstreamCombiner PROPERTIES OUTPUT_NAME bitstreamCombiner)
