#pragma once
#include "position.hpp"

void generateEndgamesTable(unsigned maxStonesInPlay);

unsigned indexFromPositionCheck(Position& position, unsigned player, unsigned maxInPlay);

void positionFromIndex(unsigned index, Position& position, unsigned& player);

void generateAllPositions(unsigned stonesInPlay);

#define UNKNOWN 0
#define PLAYER1_WIN 1
#define PLAYER2_WIN 2
#define DRAW 3

void reserveRequiredTableSize(unsigned stonesInPlay);

unsigned lookupEndgamePosition(Position& position, unsigned player);
void setEndgameIndex(unsigned index, unsigned value);