cmake_minimum_required(VERSION 3.5)
project(networkWrapper)

# set(CMAKE_C_FLAGS "-std=c++11")

find_package(Boost COMPONENTS system filesystem thread REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)


add_subdirectory(src/main)
