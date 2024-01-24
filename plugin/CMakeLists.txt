if(TARGET_STEAM_JP)
   add_compile_definitions(NINJA_GAIDEN_SIGMA_2_STEAM_JP=1)
else()
   add_compile_definitions(NINJA_GAIDEN_SIGMA_2_STEAM_AE=1)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(core)
block()
   set(CMAKE_SHARED_MODULE_PREFIX "nm")
   add_subdirectory(effect)
endblocK()

