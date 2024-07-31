# NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
# is marked with CC0 1.0. This file is a part of NINJA GAIDEN
# Master Collection NM Plugin.

add_library(util STATIC util/util.cpp)
target_include_directories(util INTERFACE util)
target_link_libraries(util distormx)

add_subdirectory(plugin/core)
add_subdirectory(plugin/loader)
add_subdirectory(plugin/effect)

install(TARGETS plugin_core DESTINATION install)
install(TARGETS plugin_loader DESTINATION install/plugin)
install(TARGETS plugin_effect DESTINATION install/plugin)
