cmake_minimum_required(VERSION 3.10)
project(MiniOS)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include_directories(src)

add_executable(MiniOS src/main.cpp
                      src/memory/memory_manager.cpp
                      src/process/scheduler.cpp
                      src/sync/semaphore.cpp
                      src/sync/producer_consumer.cpp
                      src/filesystem/filesystem.cpp
) 

target_link_libraries(MiniOS Threads::Threads)
