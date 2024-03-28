# NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
# is marked with CC0 1.0. This file is a part of NINJA GAIDEN
# Master Collection NM Plugin.

add_library(plugin_core MODULE core.cpp
			       qol.cpp)
target_link_libraries(plugin_core util)
target_link_options(plugin_core PRIVATE -static)
set_target_properties(plugin_core PROPERTIES
		      PREFIX ""
		      OUTPUT_NAME "dbghelp"
		      NO_SONAME on)
