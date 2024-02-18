# NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0.
# This file is a part of NGS2 NM Plugin.

add_library (gore OBJECT gore/gore.cpp
			 gore/crush.cpp
		         gore/mutil.cpp)
target_link_libraries(gore util)

add_library(plugin_effect MODULE effect.cpp)
target_link_libraries(plugin_effect gore)
set_target_properties(plugin_effect PROPERTIES PREFIX "nm" OUTPUT_NAME "effect")
target_link_options(plugin_effect PRIVATE -static)
