add_library(Concurrency
        src/SemaphoreGuard.cc
        src/Thread.cc
        src/linux/Notepad.cc
        src/linux/Semaphore.cc
        src/linux/Sleep.cc
        src/linux/Thread.cc
        )
include_directories(. ../exception ../type ../../service/logger)
link_libraries(Concurrency Exception pth)
