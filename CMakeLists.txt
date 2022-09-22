project(HelloWorld)
cmake_minimum_required(VERSION 3.0)

add_executable(mancalaBot main.cpp)

set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -g")

target_link_libraries(mancalaBot PUBLIC endgames utility minimax)

add_subdirectory(endgames)
add_subdirectory(utility)
add_subdirectory(minimax)
