if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
add_library(map
src/linux/Lookup.c)
target_include_directories(map PUBLIC .)
target_link_libraries(map mem)
endif()
