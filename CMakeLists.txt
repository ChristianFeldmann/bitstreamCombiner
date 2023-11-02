cmake_minimum_required(VERSION 3.14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(bitstreamCombiner)

option(ENABLE_TEST "Enable build of Tests" OFF)

add_subdirectory(src)
add_subdirectory(app)

if(ENABLE_TEST)
    message(STATUS "Enable building of unit tests")
    enable_testing()
    add_subdirectory(test/unit)
else()
    message(STATUS "Not building unit tests")
endif()
