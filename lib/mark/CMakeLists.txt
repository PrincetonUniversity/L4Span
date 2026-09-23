set(SOURCES mark_factory.cpp)

add_library(paws_mark STATIC ${SOURCES})
target_link_libraries(paws_mark)

add_subdirectory(ring_buffer)