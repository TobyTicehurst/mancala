#include "endgames.hpp"
#include "minimax.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <cstdio>

#define _GNU_SOURCE



unsigned indexFromPosition(Position& position, unsigned player)
{
    unsigned index = 2;

    index <<= position.slots[12];
    index += 1;

    // Playing slots (not goals)
    for (size_t i = 11; i > 6; i--)
    {
        index <<= position.slots[i] + 1;
        index += 1;
    }
    for (size_t i = 5; i > 0; i--)
    {
        index <<= position.slots[i] + 1;
        index += 1;
    }
    index <<= position.slots[0];

    // Goal 1 (5 bits)
    index <<= 5;
    // double check goal 1 is only 5 bits
    unsigned goal1 = position.slots[6] & 0x1F;
    index |= goal1;

    // whos turn is it?
    index <<= 1;
    index |= player;

    return index;
}

unsigned indexFromPositionCheck(Position& position, unsigned player, unsigned stonesInPlay)
{
    // ensure sum of stones is 48
    unsigned sumInPlay = 0U;
    for (size_t i = 0U; i < 6U; i++)
    {
        sumInPlay += position.slots[i];
    }

    for (size_t i = 7U; i < 13U; i++)
    {
        sumInPlay += position.slots[i];
    }

    if (sumInPlay > stonesInPlay)
    {
        printf("Error: too many stones in play: %d\n", sumInPlay);
        exit(-1);
    }

    unsigned player1Goal = position.slots[6U];
    unsigned player2Goal = position.slots[13U];
    unsigned sumOfStones = sumInPlay + player1Goal + player2Goal;
    if (sumOfStones != 48U)
    {
        printf("Error: incorrect number of stones on board: %d\n", player1Goal);
        exit(-1);
    }

    if (player1Goal > 24U || player2Goal > 24U)
    {
        printf("Error: too many stones in goals: %d, %d\n", player1Goal, player2Goal);
        exit(-1);
    }

    if (player > 1)
    {
        printf("Error: incorrect player value: %d\n", player);
        exit(-1);
    }

    return indexFromPosition(position, player);
}

static unsigned findFirstSetBit(unsigned word)
{
    unsigned firstSetBit = ffsl(word);
    if (firstSetBit == 0)
    {
        printf("Error: couldn't find next set bit\n");
        exit(-1);
    }
    return firstSetBit;
}

void positionFromIndex(unsigned index, Position& position, unsigned& player)
{
    unsigned firstSetBit;

    // player turn (1 bit)
    player = index & 0x1U;
    index >>= 1U;

    // player 1 goal (5 bits)
    position.slots[6U] = index & 0x1FU;
    index >>= 5U;

    // player 1 slots
    for (size_t i = 0U; i < 6U; i++)
    {
        firstSetBit = findFirstSetBit(index);
        position.slots[i] = firstSetBit - 1U;
        index >>= firstSetBit;
    }

    // player 2 slots
    for (size_t i = 7; i < 12; i++)
    {
        firstSetBit = findFirstSetBit(index);
        position.slots[i] = firstSetBit - 1U;
        index >>= firstSetBit;
    }

    position.slots[12] = findFirstSetBit(index) - 1U;

    unsigned totalStones = 0U;
    for (size_t i = 0U; i < 13U; i++)
    {
        totalStones += position.slots[i];
    }

    if (totalStones > 48U)
    {
        printf("Error: too many stones: %d\n", totalStones);
        exit(-1);
    }

    position.slots[13] = 48U - totalStones;
}

static inline void generatePlayer2Slots(
    Position& position, 
    unsigned remainder, 
    size_t slot, 
    unsigned stonesInPlay)
{
    // if we have reached the end of player 2's slots
    if (slot == 12)
    {
        // put the remaining stones here
        position.slots[slot] = remainder;
        // for now just print the positions
        // PrintPosition(position);
        int evaluationPlayer1 = minimaxNoDepth(position, -100, 100, PLAYER1);
        unsigned valuePlayer1;
        if (evaluationPlayer1 == 50)
            valuePlayer1 = PLAYER1_WIN;
        else if (evaluationPlayer1 == -50)
            valuePlayer1 = PLAYER2_WIN;
        else if (evaluationPlayer1 == 0)
            valuePlayer1 = DRAW;
        else
        {
            printf("Error: Invalid evaluation player 1: %d\n", evaluationPlayer1);
            PrintPosition(position);
            exit(-1);
        }
        
        unsigned indexPlayer1 = indexFromPositionCheck(position, PLAYER1, stonesInPlay);
        setEndgameIndex(indexPlayer1, valuePlayer1);

        int evaluationPlayer2 = minimaxNoDepth(position, -100, 100, PLAYER2);
        unsigned valuePlayer2;
        if (evaluationPlayer2 == 50)
            valuePlayer2 = PLAYER1_WIN;
        else if (evaluationPlayer2 == -50)
            valuePlayer2 = PLAYER2_WIN;
        else if (evaluationPlayer2 == 0)
            valuePlayer2 = DRAW;
        else
        {
            printf("Error: Invalid evaluation player 2: %d\n", evaluationPlayer2);
            PrintPosition(position);
            exit(-1);
        }
        unsigned indexPlayer2 = indexFromPositionCheck(position, PLAYER2, stonesInPlay);
        
        setEndgameIndex(indexPlayer2, valuePlayer2);
    }
    else
    {
        for (unsigned slotStones = 0; slotStones <= remainder; slotStones++)
        {
            position.slots[slot] = slotStones;
            // generate next slot
            generatePlayer2Slots(position, remainder - slotStones, slot + 1, stonesInPlay);
        }
    }
}

static inline void generatePlayer1Slots(
    Position& position, 
    unsigned remainder, 
    size_t slot, 
    unsigned player2Stones, 
    unsigned stonesInPlay)
{
    // if we have reached the end of player 1's slots
    if (slot == 5)
    {
        // put the remaining stones here
        position.slots[slot] = remainder;
        // move on to player 2's slots
        generatePlayer2Slots(position, player2Stones, 7, stonesInPlay);
    }
    else
    {
        for (unsigned slotStones = 0; slotStones <= remainder; slotStones++)
        {
            position.slots[slot] = slotStones;
            // generate next slot
            generatePlayer1Slots(position, remainder - slotStones, slot + 1, player2Stones, stonesInPlay);
        }
    }
}

void generateAllPositions(unsigned stonesInPlay)
{
    reserveRequiredTableSize(stonesInPlay);

    Position position;
    for (unsigned player1Stones = 1; player1Stones < stonesInPlay; player1Stones++)
    {
        unsigned player2Stones = stonesInPlay - player1Stones;
        unsigned player1Remainder = player1Stones;

        unsigned stonesInGoals = 48 - stonesInPlay;
        for (unsigned stonesInGoal1 = 24 - stonesInPlay; stonesInGoal1 <= 24; stonesInGoal1++)
        {
            unsigned stonesInGoal2 = 48 - stonesInPlay - stonesInGoal1;
            position.slots[PLAYER1_GOAL] = stonesInGoal1;
            position.slots[PLAYER2_GOAL] = stonesInGoal2;
            generatePlayer1Slots(position, player1Stones, 0, player2Stones, stonesInPlay);
        }
    }
}

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
