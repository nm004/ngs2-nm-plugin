# NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0.
# This file is a part of NGS2 NM Plugin.

cmake_minimum_required(VERSION 3.20)
project(ngs2-nm-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32)
set(CMAKE_OPTIMIZE_DEPENDENCIES on)
set(CMAKE_SHARED_MODULE_SUFFIX ".dll")

add_subdirectory(distormx)
add_subdirectory(src)

install(FILES doc/README.txt DESTINATION /)
