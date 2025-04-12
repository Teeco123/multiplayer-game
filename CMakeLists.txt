cmake_minimum_required(VERSION 3.20)
project(Server)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

file(GLOB HEADERS include/*.hpp)
file(GLOB SOURCES src/*.cpp)

add_executable(Server server.cpp ${INCLUDE} ${SOURCES})

set_target_properties(Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
