# NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
# is marked with CC0 1.0. This file is a part of NINJA GAIDEN
# Master Collection NM Plugin.

add_library(plugin_steam_bugfixes MODULE steam-bugfixes.cpp)
target_link_libraries(plugin_steam_bugfixes util)
target_link_options(plugin_steam_bugfixes PRIVATE -static)
set_target_properties(plugin_steam_bugfixes PROPERTIES PREFIX "nm" OUTPUT_NAME "steam-bugfixes")
