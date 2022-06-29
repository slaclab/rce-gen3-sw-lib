add_library(memory
  src/Arena.cc
  src/FreeList.cc
)
target_include_directories(memory PUBLIC .)
target_link_libraries(memory Debug container)

if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )

add_library(mem
  src/armCA9/memRegion.c
  src/armCA9/memResourceHelpers.S
  src/armCA9/memFlagsTable.c
  src/armCA9/memResource.c
  src/armCA9/memCache.c
  src/armCA9/memMmu.c
  src/armCA9-linux/memMapDeviceRegisters.c
)
target_include_directories(mem PUBLIC .)
target_link_libraries(mem Debug)
endif()

