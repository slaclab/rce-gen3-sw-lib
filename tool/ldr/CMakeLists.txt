if( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" )

add_library(ldr
src/Ldr.c
)

target_include_directories(ldr PUBLIC .)
target_include_directories(ldr PRIVATE ../../configuration/system/)
target_link_libraries(ldr kvt mem elf)
endif()
