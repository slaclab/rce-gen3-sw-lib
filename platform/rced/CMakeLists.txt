if( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" )

list (APPEND libs 
	csi 
	memory 
	concurrency 
	exception 
	Debug 
	string io 
	map udp net 
	dsl_l atca_l 
	dslutil hash 
	kvt)

add_executable(rced src/rced.cc)
target_link_libraries(rced ${libs})

add_executable(sysinfo src/rced.cc)
target_link_libraries(sysinfo ${libs})
endif()

