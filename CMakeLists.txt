# NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
# is marked with CC0 1.0. This file is a part of NINJA GAIDEN
# Master Collection NM Plugin.

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

link_libraries(-static util)

add_subdirectory(src/core)
add_subdirectory(src/steam-bugfixes)
add_subdirectory(src/loader)
add_subdirectory(src/effect)

set_property(TARGET core PROPERTY PREFIX "")
set_property(TARGET core PROPERTY OUTPUT_NAME "dbghelp")

set_property(TARGET
	effect
	loader
	steam-bugfixes
	PROPERTY PREFIX "nm")

install(TARGETS core DESTINATION install)
install(TARGETS steam-bugfixes DESTINATION install/plugin)
install(TARGETS loader DESTINATION install/plugin)
install(TARGETS effect DESTINATION install/plugin)
