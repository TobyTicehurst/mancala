#include "endgamesTable.hpp"
#include "endgames.hpp"
#include <vector>
#include <cstdio>
#include <stdlib.h>


#define BITS_FOR_GOAL 5
#define BITS_FOR_PLAYER 1
#define BITS_FOR_STONES 12

std::vector<unsigned> endgamesTable;
unsigned maxStonesInPlay;

void reserveRequiredTableSize(unsigned stonesInPlay)
{
    maxStonesInPlay = stonesInPlay;
    unsigned power = BITS_FOR_GOAL + BITS_FOR_PLAYER + BITS_FOR_STONES + stonesInPlay;
    if (power > 31)
    {
        printf("Error: Overflowing an unsigned\n");
        exit(-1);
    }

    endgamesTable.clear();
    endgamesTable.reserve(1 << power);
}

void setEndgameIndex(unsigned index, unsigned value)
{
    endgamesTable[index] = value;
}

unsigned lookupEndgamePosition(Position& position, unsigned player)
{
    unsigned stonesInPlay = 48 - position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    if (stonesInPlay <= maxStonesInPlay)
    {
        unsigned index = indexFromPositionCheck(position, player, maxStonesInPlay);
        return endgamesTable[index];
    }
    
    return UNKNOWN;
}