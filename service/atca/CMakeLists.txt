set(rce_src "")
set(rce_libs "")
if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
  set(rce_src src/init.cc)
  set(rce_libs bsi)
endif()


add_library(atca_l
src/Address.cc
src/Attributes.cc
src/Client.cc
${rce_src}
src/Space.cc
)

target_include_directories(atca_l PUBLIC .)
target_link_libraries(atca_l net dsl_l conversion container csi ${rce_libs} memory)
if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
target_link_libraries(atca_l elf)
endif()
