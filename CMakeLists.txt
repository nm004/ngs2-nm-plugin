cmake_minimum_required(VERSION 3.20)
project(ngs2-nm-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32)
set(CMAKE_SHARED_MODULE_PREFIX "nm")
set(CMAKE_OPTIMIZE_DEPENDENCIES on)

find_package(PolyHook_2 CONFIG REQUIRED)

add_subdirectory(core)
add_subdirectory(effect)

install(TARGETS dbghelp DESTINATION install)
install(TARGETS effect DESTINATION install/plugin)
install(FILES docs/README.txt DESTINATION /)
