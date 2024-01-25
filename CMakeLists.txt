cmake_minimum_required(VERSION 3.20)
project(ngs2-nm-plugin LANGUAGES CXX)

find_package(PolyHook_2 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32)
set(CMAKE_OPTIMIZE_DEPENDENCIES on)

if(TARGET_STEAM_JP)
   add_compile_definitions(NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_JP=1)
elseif(TARGET_STEAM_AE)
   add_compile_definitions(NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_AE=1)
endif()

add_subdirectory(plugin)

install(TARGETS core DESTINATION install)
install(TARGETS effect DESTINATION install/plugin)
install(FILES docs/README.txt DESTINATION /)
