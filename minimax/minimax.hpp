#pragma once
#include "position.hpp"
#include "endgamesTable.hpp"


struct MemoryNode
{
    MemoryNode* children;
    int evaluation;
};

/* minimax functions */
int minimax                     (Position& position, int alpha, int beta, unsigned maximizingPlayer, unsigned depth);
int minimaxNoDepth              (Position& position, int alpha, int beta, unsigned maximizingPlayer);
int minimaxEndgameTable         (Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table, unsigned depth);
int minimaxEndgameTableNoDepth  (Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table);

int minimaxMemoryEndgameTableNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table, MemoryNode* node, unsigned depth);

/* MTD(F) functions */
int mtdfNoMemory                   (Position& position, int initialGuess, unsigned maximizingPlayer, unsigned depth);
int mtdfNoMemoryNoDepth            (Position& position, int initialGuess, unsigned maximizingPlayer);
int mtdfNoMemoryEndgameTable       (Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table, unsigned depth);
int mtdfNoMemoryEndgameTableNoDepth(Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table);

int mtdfEndgameTableNoDepth(Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table, MemoryNode* root, int depth);
