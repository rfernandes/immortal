cmake_minimum_required(VERSION 3.0)

project(ascention)

add_definitions("-std=c++17 -Wall -Wextra -pedantic")

add_executable(ascention
main.cpp
Input.cpp
Output.cpp
)

install(TARGETS ascention RUNTIME DESTINATION bin)
