FILE(GLOB_RECURSE SOURCE_FILES *.cpp)
FILE(GLOB_RECURSE HEADER_FILES *.h)

add_executable(bitstreamCombiner ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(bitstreamCombiner PRIVATE ${CMAKE_SOURCE_DIR}/src)

set_target_properties(bitstreamCombiner PROPERTIES OUTPUT_NAME bitstreamCombiner)
