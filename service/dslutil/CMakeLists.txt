if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
list(APPEND rce_libs
	 src/reboot_init.cc
         src/update_init.cc
         src/id_init.cc
         src/armCA9-linux/RebootSpace.cc
         src/armCA9-linux/UpdateSpace.cc
         src/armCA9-linux/IdSpace.cc
	 )
else()
list(APPEND rce_libs
  src/i86-linux/RebootSpace.cc
  src/i86-linux/UpdateSpace.cc
  src/i86-linux/IdSpace.cc
  )
endif()

add_library(dslutil
src/RebootBase.cc
src/RebootClient.cc
src/RebootSpace.cc
src/UpdateBase.cc
src/UpdateClient.cc
src/UpdateListener.cc
src/UpdateSpace.cc
src/IdBase.cc
src/IdClient.cc
src/IdListener.cc
src/IdSpace.cc
src/AddressStar.cc
src/Base.cc
${rce_libs}
)


target_link_libraries(dslutil atca_l cob_ipmc dsl_l net conversion time)
target_include_directories(dslutil PUBLIC .)


add_executable(dsl_reboot src/linux/dsl_reboot.cc)
target_link_libraries(dsl_reboot dslutil)
add_executable(dsl_identify src/linux/dsl_identify.cc)
target_link_libraries(dsl_identify dslutil)
#add_executable(dsl_update src/armCA9-linux/dsl_update.cc) #arm
add_executable(dsl_update src/i86-linux/dsl_update.cc)
target_link_libraries(dsl_update dslutil)
