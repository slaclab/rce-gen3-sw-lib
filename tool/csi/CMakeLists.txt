add_library(csi
src/Proxy.cc
)
target_link_libraries(csi container memory)
target_include_directories(csi PUBLIC .)
