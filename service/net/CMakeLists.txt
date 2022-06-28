add_library(net
src/Endpoint.cc            
src/MacAddress.cc             
src/Socket.cc
src/Interface.cc 
)

target_link_libraries(net conversion)
target_include_directories(net PUBLIC .)
