cmake_minimum_required(VERSION 3.20)
project(Game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/raylib)

file(GLOB HEADERS include/*.hpp)
file(GLOB SOURCES src/*.cpp)

add_executable(Server server.cpp ${INCLUDE} ${SOURCES})
add_executable(Client client.cpp ${INCLUDE} ${SOURCES})

set_target_properties(Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/server
)

target_link_libraries(Client raylib)
set_target_properties(Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/client
)
