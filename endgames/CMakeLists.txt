
add_library(endgames STATIC endgames.cpp threadedEndgames.cpp endgamesTreeTable.cpp)
target_link_libraries(endgames PUBLIC utility minimax)
target_include_directories(endgames PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} minimax)