add_library(string
src/id2node.cc  src/ip2dot.cc  src/mac2colon.cc  src/Name.cc  src/StringBuffer.cc
)
target_include_directories(string PUBLIC .)
