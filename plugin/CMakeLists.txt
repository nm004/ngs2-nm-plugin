include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(core)
block()
   set(CMAKE_SHARED_MODULE_PREFIX "nm")
   add_subdirectory(effect)
endblocK()

