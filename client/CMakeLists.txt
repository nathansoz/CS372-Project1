cmake_minimum_required(VERSION 2.8.11)
project(client)

SET(CMAKE_CXX_FLAGS "-std=c++11")

set(SOURCE_FILES src/main.cpp include/ChatClient.h include/Constants.h src/ChatClient.cpp)
add_executable(client ${SOURCE_FILES})

target_link_libraries(client pthread readline ncurses)
