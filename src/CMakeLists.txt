FILE(GLOB_RECURSE SOURCE_FILES *.cpp)
FILE(GLOB_RECURSE HEADER_FILES *.h)

add_library(bitstreamCombinerLib STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(bitstreamCombinerLib PRIVATE ${CMAKE_SOURCE_DIR}/src)

set_target_properties(bitstreamCombinerLib PROPERTIES OUTPUT_NAME bitstreamCombiner)
