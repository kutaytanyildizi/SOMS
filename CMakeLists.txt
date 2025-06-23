cmake_minimum_required(VERSION 3.29.5)
project(SOMS)

set(CMAKE_CXX_STANDARD 20)


add_executable(SOMS main.cpp src/TCPServer.cpp )

target_link_libraries(SOMS
  ws2_32
)

target_include_directories(SOMS PRIVATE 
    ${CMAKE_SOURCE_DIR}/defines
    ${CMAKE_SOURCE_DIR}/include
)

