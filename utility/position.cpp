#include "position.hpp"
#include <cstring>
#include <cstdio>


void PrintPosition(Position& position)
{
    printf("    [%02d] [%02d] [%02d] [%02d] [%02d] [%02d]    \n", 
        position.slots[12], 
        position.slots[11], 
        position.slots[10], 
        position.slots[9], 
        position.slots[8], 
        position.slots[7]);
    
    printf("[%02d]                             [%02d]\n", position.slots[13], position.slots[6]);

    printf("    [%02d] [%02d] [%02d] [%02d] [%02d] [%02d]    \n\n", 
        position.slots[0], 
        position.slots[1], 
        position.slots[2], 
        position.slots[3], 
        position.slots[4], 
        position.slots[5]);

    fflush(stdout);
}

static inline unsigned getGoal(bool player)
{
    //return player ? PLAYER2_GOAL : PLAYER1_GOAL;
    return PLAYER1_GOAL + player * (PLAYER2_GOAL - PLAYER1_GOAL);
}

static inline unsigned player1Stones(const Position& position)
{
    return position.slots[0] + 
           position.slots[1] + 
           position.slots[2] + 
           position.slots[3] + 
           position.slots[4] + 
           position.slots[5];
}

static inline unsigned player2Stones(const Position& position)
{
    return position.slots[7] + 
           position.slots[8] + 
           position.slots[9] + 
           position.slots[10] + 
           position.slots[11] + 
           position.slots[12];
}

unsigned long numPositions[48];

unsigned generateChildPosition(const Position& position, Position& childPosition, unsigned move, bool player, bool& gameOver)
{
    gameOver = false;

    memcpy(childPosition.slots, position.slots, sizeof(childPosition.slots));
    unsigned numPips = childPosition.slots[move];

    childPosition.slots[move] = 0;
    unsigned endSlot;
    
    // if player 2
    if (player)
    {
        // move is 12, i = 13; i <= 20
        unsigned final = move + numPips;
        for (unsigned i = move + 1; i < final; ++i)
        {
            // skip player 1's goal
            unsigned slot = (((i + 6) % (NUM_SLOTS - 1U)) + 7U) % NUM_SLOTS;
            ++childPosition.slots[slot];
        }
        endSlot = (((final + 6) % (NUM_SLOTS - 1U)) + 7U) % NUM_SLOTS;
        ++childPosition.slots[endSlot];
    }
    // if player 1
    else
    {
        // 1. add 1 from move+1 to end
        // 2. add 1 from start to end
        // 3. skip opponent's goal
        unsigned final =  move + numPips;
        for (unsigned i = move + 1; i < final; ++i)
        {
            // skip player 2's goal
            unsigned slot = i % (NUM_SLOTS - 1);
            ++childPosition.slots[slot];
        }
        endSlot = final % (NUM_SLOTS - 1);
        ++childPosition.slots[endSlot];
    }

    // if end slot is the players goal, give the player an extra turn
    unsigned playerGoal = getGoal(player);
    unsigned playAgain = (endSlot == playerGoal);

    // if ((childPosition.slots[endSlot] == 1))
    // {
    //     if (player)
    //     {
    //         if ((endSlot >= 7) && (endSlot <= 12))
    //         {
    //             unsigned oppositeSlot = 12 - endSlot;
    //             unsigned pipsToSteal = childPosition.slots[oppositeSlot];
    //             childPosition.slots[oppositeSlot] = 0;
    //             childPosition.slots[PLAYER2_GOAL] += pipsToSteal;
    //         }
    //     }
    //     else
    //     {
    //         if (endSlot <= 5)
    //         {
    //             unsigned oppositeSlot = 12 - endSlot;
    //             unsigned pipsToSteal = childPosition.slots[oppositeSlot];
    //             childPosition.slots[oppositeSlot] = 0;
    //             childPosition.slots[PLAYER1_GOAL] += pipsToSteal;
    //         }
    //     }
    // }
    // stealing rules (TODO do you steal if you loop the whole way around to the start slot? ie if you have 13 stones)
    if (((player == PLAYER1) && (endSlot >= 0) && (endSlot <= 5) && (childPosition.slots[endSlot] == 1)) ||
        ((player == PLAYER2) && (endSlot >= 7) && (endSlot <= 12) && (childPosition.slots[endSlot] == 1)))
    {
        unsigned oppositeSlot = 12 - endSlot;
        unsigned pipsToSteal = childPosition.slots[oppositeSlot];
        childPosition.slots[oppositeSlot] = 0;
        childPosition.slots[playerGoal] += pipsToSteal;
    }

    // if one side now has no stones left the game is over (TODO could return flag?)
    // may not need to memset as we don't care about the child position if the game is over
    unsigned player1Total = player1Stones(childPosition);
    unsigned player2Total = player2Stones(childPosition);
    if (player1Total == 0)
    {
        gameOver = true;
        childPosition.slots[PLAYER2_GOAL] += player2Total;
        memset(&childPosition.slots[7], 0, sizeof(childPosition.slots[0]) * 6);
    }
    else if (player2Total == 0)
    {
        gameOver = true;
        childPosition.slots[PLAYER1_GOAL] += player1Total;
        memset(&childPosition.slots[0], 0, sizeof(childPosition.slots[0]) * 6);
    }

    return playAgain;
}

void generateStartPosition(Position& position)
{
    position.slots[0] = 4;
    position.slots[1] = 4;
    position.slots[2] = 4;
    position.slots[3] = 4;
    position.slots[4] = 4;
    position.slots[5] = 4;
    position.slots[6] = 0;

    position.slots[7] = 4;
    position.slots[8] = 4;
    position.slots[9] = 4;
    position.slots[10] = 4;
    position.slots[11] = 4;
    position.slots[12] = 4;
    position.slots[13] = 0;
}

void resetPositionStats()
{
    memset(&numPositions, 0, sizeof(numPositions));
}

void printPositionStats()
{
    for (size_t i = 0; i < 48; i++)
    {
        printf("%llu, ", numPositions[i]);
    }
    printf("\n");
}
