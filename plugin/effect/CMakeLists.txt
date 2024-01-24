add_subdirectory(gore)

add_library(effect MODULE effect.cpp)
target_link_libraries(effect gore)
target_link_options(effect PRIVATE -static)
