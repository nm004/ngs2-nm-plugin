add_library(dbghelp MODULE dbghelp.cpp core.cpp loader.cpp)
target_include_directories(dbghelp PRIVATE ../include)
target_link_libraries(dbghelp -static PolyHook_2::PolyHook_2)
set_target_properties(dbghelp PROPERTIES
		      PREFIX ""
		      NO_SONAME on)
