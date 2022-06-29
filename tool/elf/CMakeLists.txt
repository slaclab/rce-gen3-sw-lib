if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )

add_library(elf
src/linker.c
src/lnkObjDb.c
src/lnkSearchDag.c
src/exidx.c
src/linux/exidxTable.cc
)

target_include_directories(elf PUBLIC .)
target_include_directories(elf PUBLIC ../../configuration/system/)

target_link_libraries(elf Debug ldr svt concurrency)
endif()
