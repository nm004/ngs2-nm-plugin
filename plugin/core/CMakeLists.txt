add_library(core MODULE dbghelp.cpp core.cpp loader.cpp)
target_link_libraries(core PolyHook_2::PolyHook_2)
target_link_options(core PRIVATE -static)
set_target_properties(core PROPERTIES
		      PREFIX ""
		      OUTPUT_NAME "dbghelp"
		      NO_SONAME on)
