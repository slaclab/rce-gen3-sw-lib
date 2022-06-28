add_library(concurrency
        src/SemaphoreGuard.cc
        src/Thread.cc
        src/linux/Notepad.cc
        src/linux/Semaphore.cc
        src/linux/Sleep.cc
        src/linux/Thread.cc
        )
target_link_libraries(concurrency exception type logger pth)
target_include_directories(concurrency PUBLIC .)
