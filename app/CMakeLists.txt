
add_executable(bitstreamCombiner main.cpp)

target_include_directories(bitstreamCombiner PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bitstreamCombiner bitstreamCombinerLib)

set_target_properties(bitstreamCombiner PROPERTIES OUTPUT_NAME bitstreamCombiner)
