add_library(kvt
src/Kvt.c
)
target_include_directories(kvt PUBLIC .)
target_link_libraries(kvt list hash)
