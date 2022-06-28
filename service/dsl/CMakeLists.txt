add_library(dsl_l
src/Location.cc
src/Proxy.cc
src/Server.cc
src/Client.cc
src/linux/init.cc
src/linux/Server.cc
)
target_link_libraries(dsl_l logger exception logger concurrency conversion container csi kvt memory net udp)
target_include_directories(dsl_l PUBLIC .)

