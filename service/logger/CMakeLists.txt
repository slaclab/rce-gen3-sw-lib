add_library(logger
        src/LogMessage.cc
        src/Logger.cc
        src/LoggerImpl.cc
        src/StderrLogger.cc
        src/SyslogLogger.cc
        src/linux/StderrLogger.cc
        )
target_link_libraries(logger Type Concurrency)
include_directories(
        .
        ../../tool/type
        ../../tool/exception
        ../../tool/string
        ../../tool/concurrency
        ../../tool/io
        )