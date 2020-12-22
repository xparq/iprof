cmake_minimum_required(VERSION 3.13)

add_library(iprof STATIC
    iprof.cpp
)

if (BUILD_TEST)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    add_executable(iprof-test
        test.cpp
    )
    target_link_libraries(iprof-test PRIVATE
        iprof
        Threads::Threads
    )
endif()
