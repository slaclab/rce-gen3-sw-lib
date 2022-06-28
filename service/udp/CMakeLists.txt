add_library(udp
src/Link.cc
)
target_link_libraries(udp net csi)
target_include_directories(udp PUBLIC .)
