add_library(exception
        src/Exception.cc
        src/Exceptions.cc
        )
target_link_libraries(exception logger)
target_include_directories(exception PUBLIC .)