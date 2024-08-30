# NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0.
# This file is a part of NGS2 NM Plugin.

cmake_minimum_required(VERSION 3.20)
project(ngs2-nm-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32)
set(CMAKE_OPTIMIZE_DEPENDENCIES on)
set(CMAKE_SHARED_MODULE_SUFFIX ".dll")

add_subdirectory(distormx)

add_library(util INTERFACE)
target_include_directories(util INTERFACE src)
target_link_libraries(util INTERFACE distormx)

add_subdirectory(src/core)
add_subdirectory(src/steam-bugfixes)
add_subdirectory(src/loader)

install(TARGETS plugin_core DESTINATION install)
install(TARGETS plugin_steam_bugfixes DESTINATION install/plugin)
install(TARGETS plugin_loader DESTINATION install/plugin)
#install(TARGETS plugin_effect DESTINATION install/plugin)
install(FILES doc/README.txt DESTINATION /)
