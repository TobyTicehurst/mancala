#pragma once

#define NUM_SLOTS 14
#define PLAYER1_GOAL 6
#define PLAYER2_GOAL 13

#define PLAYER1 0
#define PLAYER2 1

typedef struct
{
    unsigned slots[NUM_SLOTS];
} 
Position;

void PrintPosition(Position& position);
void generateStartPosition(Position& position);

unsigned generateChildPosition(const Position& position, Position& childPosition, unsigned move, bool player, bool& gameOver);

void resetPositionStats();

void printPositionStats();
