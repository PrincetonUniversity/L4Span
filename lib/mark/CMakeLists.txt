set(SOURCES mark_factory.cpp)

add_library(paws_mark STATIC ${SOURCES})
target_link_libraries(paws_mark)