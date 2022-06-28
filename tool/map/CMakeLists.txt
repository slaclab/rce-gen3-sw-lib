if( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" )
add_library(map
src/linux/Lookup.c)
target_include_directories(map PUBLIC .)
target_link_libraries(map mem)
endif()
