# NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
# is marked with CC0 1.0. This file is a part of NINJA GAIDEN
# Master Collection NM Plugin.

add_library(plugin_loader MODULE loader.cpp)
target_link_libraries(plugin_loader util)
target_link_options(plugin_loader PRIVATE -static)
set_target_properties(plugin_loader PROPERTIES PREFIX "nm" OUTPUT_NAME "loader")
