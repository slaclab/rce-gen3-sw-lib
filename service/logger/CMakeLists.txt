add_library(logger
        src/LogMessage.cc
        src/Logger.cc
        src/LoggerImpl.cc
        src/StderrLogger.cc
        src/SyslogLogger.cc
        src/linux/StderrLogger.cc
        )
target_link_libraries(logger exception concurrency string io)
target_include_directories(logger PUBLIC .)
