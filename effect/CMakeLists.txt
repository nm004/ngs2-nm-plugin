add_subdirectory(gore)

add_library(effect MODULE effect.cpp)
target_include_directories(effect PRIVATE ../include)
target_link_libraries(effect -static gore)
