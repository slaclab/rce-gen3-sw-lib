add_library(Exception
        src/Exception.cc
        src/Exceptions.cc
        )
include_directories(
        .
        ../../service/logger
        )