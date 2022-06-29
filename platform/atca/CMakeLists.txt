list(APPEND libs 
	csi 
	memory 
	concurrency 
	exception 
	debug 
	kvt 
	hash 
	string 
	io 
	udp 
	net 
	logger 
	dsl_l 
	atca_l
	pthread
	)

if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
set(rce_libs mem map)
endif()

add_executable(atca_ip src/atca_ip.cc)
target_link_libraries(atca_ip ${libs} ${rce_libs})

add_executable(atca_mac src/atca_mac.cc)
target_link_libraries(atca_mac ${libs} ${rce_libs})

add_executable(atca_dump src/atca_dump.cc)
target_link_libraries(atca_dump ${libs} ${rce_libs})

add_executable(shelf_ping src/shelf_ping.cc)
target_link_libraries(shelf_ping ${libs} ${rce_libs})

add_executable(remote_service src/remote_service.cc)
target_link_libraries(remote_service atca_l ${libs} ${rce_libs})

if( "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm*" )
add_executable(atca_server src/atca_server.cc)
target_link_libraries(atca_server ${libs} ${rce_libs})

add_executable(shelf_ping_server src/shelf_ping_server.cc)
target_link_libraries(shelf_ping_server atca_l ${libs} ${rce_libs})
install(TARGETS atca_ip atca_mac atca_dump shelf_ping remote_service DESTINATION bin)

add_executable(remote_server src/remote_server.cc)
target_link_libraries(remote_server ${libs} ${rce_libs})
install(TARGETS atca_server shelf_ping_server remote_server  DESTINATION sbin)
endif()


