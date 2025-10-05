set(SOURCES mru_packing.cpp)

add_library(srsran_mru STATIC ${SOURCES})
target_link_libraries(srsran_mru srsran_support)