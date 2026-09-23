set(SOURCES ring_buffer.cpp)

add_library(ring_buffer STATIC ${SOURCES})
target_link_libraries(ring_buffer)

