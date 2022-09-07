#pragma once
#include "position.hpp"

int minimax(Position& position, unsigned depth, int alpha, int beta, unsigned maximizingPlayer);

int minimaxNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer);

unsigned isGameOver(Position& position);

void printStats();
void resetStats();