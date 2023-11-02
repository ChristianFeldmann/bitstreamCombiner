include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FILE(GLOB_RECURSE TEST_UNIT_SOURCE_FILES *.cpp)
FILE(GLOB_RECURSE TEST_UNIT_HEADER_FILES *.h)

add_executable(unitTest ${TEST_UNIT_SOURCE_FILES} ${TEST_UNIT_HEADER_FILES})

target_include_directories(unitTest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(unitTest GTest::gtest_main bitstreamCombinerLib)

include(GoogleTest)
gtest_discover_tests(unitTest)
