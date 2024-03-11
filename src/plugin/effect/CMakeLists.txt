# NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0.
# This file is a part of NGS2 NM Plugin.

add_library(plugin_effect MODULE effect.cpp
				 hit.cpp
				 mutil.cpp
				 crush.cpp)
target_link_libraries(plugin_effect util)
set_target_properties(plugin_effect PROPERTIES PREFIX "nm" OUTPUT_NAME "effect")
target_link_options(plugin_effect PRIVATE -static)
