
add_library(minimax STATIC minimax.cpp)
target_link_libraries(minimax PUBLIC utility endgames)
target_include_directories(minimax PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})