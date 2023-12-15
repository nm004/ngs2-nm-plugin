add_library (gore OBJECT mutil.cpp crush.cpp misc.cpp)
target_link_libraries (gore PolyHook_2::PolyHook_2)
target_include_directories(gore PRIVATE ../../include)
