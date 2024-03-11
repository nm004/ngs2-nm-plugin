# NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0.
# This file is a part of NGS2 NM Plugin.

add_library(util util/util.cpp)
target_include_directories(util PUBLIC util)
target_link_libraries(util distormx)

add_subdirectory(plugin/core)
add_subdirectory(plugin/effect)

install(TARGETS plugin_core DESTINATION install)
install(TARGETS plugin_effect DESTINATION install/plugin)
