#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <algorithm>


// #ifndef max
// #define max(a,b)            (((a) > (b)) ? (a) : (b))
// #endif

// #ifndef min
// #define min(a,b)            (((a) < (b)) ? (a) : (b))
// #endif

// int max(int a, int b)
// {
//     return a > b ? a : b;
// }

// int min(int a, int b)
// {
//     return a < b ? a : b;
// }

int initialDepth = 0;

// start position
//      [04] [04] [04] [04] [04] [04]     
// [00]                               [00]
//      [04] [04] [04] [04] [04] [04]     

// indices of slots
//      [12] [11] [10] [09] [08] [07]     
// [13]                               [06]
//      [00] [01] [02] [03] [04] [05]     

// opposite is 12 - x

enum slotName
{
    P1First,
    P1Second,
    P1Third,
    P1Fourth,
    P1Fifth,
    P1Sixth,
    P1Goal,
    P2First,
    P2Second,
    P2Third,
    P2Fourth,
    P2Fifth,
    P2Sixth,
    P2Goal,
    NumSlots
};

void PrintPosition(Position* pos, int eval)
{
    printf("Evaluation: %d\n", eval);
    printf("    [%02d] [%02d] [%02d] [%02d] [%02d] [%02d]    \n", 
        pos->slots[P2Sixth], 
        pos->slots[P2Fifth], 
        pos->slots[P2Fourth], 
        pos->slots[P2Third], 
        pos->slots[P2Second], 
        pos->slots[P2First]);
    
    printf("[%02d]                             [%02d]\n", pos->slots[P2Goal], pos->slots[P1Goal]);

    printf("    [%02d] [%02d] [%02d] [%02d] [%02d] [%02d]    \n\n", 
        pos->slots[P1First], 
        pos->slots[P1Second], 
        pos->slots[P1Third], 
        pos->slots[P1Fourth], 
        pos->slots[P1Fifth], 
        pos->slots[P1Sixth]);

    fflush(stdout);
}

unsigned isGameOver(Position* position)
{
    return (position->slots[P1Goal] > 24 || position->slots[P2Goal] > 24);
}

// unsigned isGameOver(Position* position)
// {
//     unsigned player1Total = position->slots[0] + position->slots[1] + position->slots[2] + position->slots[3] + position->slots[4] + position->slots[5];
//     unsigned player2Total = position->slots[7] + position->slots[8] + position->slots[9] + position->slots[10] + position->slots[11] + position->slots[12];
    
//     return (player1Total == 0) || (player2Total == 0);

//     // // test first 6 bytes (miss next 2)
//     // return !((uint64_t*)position->slots)[0] & 0x0000FFFFFFFFFFFFULL || !((((uint64_t*)position->slots)[1] & 0x000000FFFFFFFFFFULL) || position->slots[7]);
// }

int staticEvaluation(Position* position)
{
    if (position->slots[P1Goal] > 24)
        return 50;
    else if (position->slots[P2Goal] > 24)
        return -50;
    else
        return position->slots[P1Goal] - position->slots[P2Goal];
}

static inline unsigned getGoal(unsigned player)
{
    // return P1Goal if player == 0 (player 1)
    // return P2Goal if player == 1 (player 2)

    // if (player)
    //     return P2Goal;
    // else
    //     return P1Goal;
    // return P1Goal + player * (P2Goal - P1Goal);
    return player ? P2Goal : P1Goal;
}

unsigned positionsGenerated = 0;
unsigned endgamePositions = 0;
static inline unsigned generateChildPosition(Position* position, Position* childPosition, unsigned move, unsigned player)
{
    // stats
    positionsGenerated++;
    if (48 - position->slots[6] - position->slots[13] < 13)
        endgamePositions++;

    memcpy(childPosition->slots, position->slots, sizeof(childPosition->slots));
    unsigned numPips = childPosition->slots[move];
    printf("num pips: %d\n", numPips);

    childPosition->slots[move] = 0;

    if (player == 0)
    {
        // 1. add 1 from move+1 to end
        // 2. add 1 from start to end
        // 3. skip opponent's goal
        for (size_t i = move + 1; i <= move + numPips; ++i)
        {
            // skip slot 13
            ++childPosition->slots[i % (NumSlots - 1)];
        }
    }
    else
    {
        for (size_t i = move + 1; i <= move + numPips; ++i)
        {
            if ((i % NumSlots) == 6)
            {
                continue;
            }
            // skip slot 6
            ++childPosition->slots[i % NumSlots];
        }
    }

    PrintPosition(*childPosition);


    // TODO check rules on stealing
    // get end
    // if end is players goal, give extra turn
    // if end is 
    unsigned endSlot = (move + numPips) % NumSlots;
    unsigned playerGoal = getGoal(player);
    unsigned opponentGoal = getGoal(!player);
    unsigned playAgain;
    if (endSlot == playerGoal)
    {
        playAgain = 1;
    }
    else
    {
        playAgain = 0;
    }

    // stealing rules
    if (((player == 0) && (endSlot >= 0) && (endSlot <= 5) && (position->slots[endSlot] == 0)) ||
        ((player == 1) && (endSlot >= 7) && (endSlot <= 12) && (position->slots[endSlot] == 0)))
    {
        unsigned oppositeSlot = 12 - endSlot;
        unsigned pipsToSteal = childPosition->slots[oppositeSlot];
        childPosition->slots[oppositeSlot] = 0;
        childPosition->slots[playerGoal] += pipsToSteal;
    }
        // old rules
        // if ((endSlot != opponentGoal) && (position->slots[endSlot] == 0))
        // {
        //     unsigned oppositeSlot = 12 - endSlot;
        //     unsigned pipsToSteal = childPosition->slots[oppositeSlot];
        //     childPosition->slots[oppositeSlot] = 0;
        //     childPosition->slots[playerGoal] += pipsToSteal;
        // }

    //     // next players turn
    //     playAgain = 0;
    // }
    // else
    // {
    //     // same players go again
    //     playAgain = 1;
    // }

    //     // play again if finishing on own goal
    // unsigned playAgain = (endSlot == playerGoal);
    // // stealing rules
    // if (!playAgain && (endSlot != opponentGoal) && (position->slots[endSlot] == 0))
    // {
    //     unsigned oppositeSlot = 12 - endSlot;
    //     unsigned pipsToSteal = childPosition->slots[oppositeSlot];
    //     childPosition->slots[oppositeSlot] = 0;
    //     childPosition->slots[playerGoal] += pipsToSteal;
    // }

    // if one side now has no pips left the game is over
    unsigned player1Total = 
        childPosition->slots[0] + 
        childPosition->slots[1] + 
        childPosition->slots[2] + 
        childPosition->slots[3] + 
        childPosition->slots[4] + 
        childPosition->slots[5];
    unsigned player2Total = 
        childPosition->slots[7] + 
        childPosition->slots[8] + 
        childPosition->slots[9] + 
        childPosition->slots[10] + 
        childPosition->slots[11] + 
        childPosition->slots[12];
    if (player1Total == 0)
    {
        childPosition->slots[13] += player2Total;
        memset(&childPosition->slots[7], 0, sizeof(childPosition->slots[0]) * 6);
    }
    else if (player2Total == 0)
    {
        childPosition->slots[6] += player1Total;
        memset(&childPosition->slots[0], 0, sizeof(childPosition->slots[0]) * 6);
    }

    return playAgain;
}

unsigned positionsEvaluated = 0;
int minimax0(Position* position, unsigned depth, int alpha, int beta, unsigned maximizingPlayer)
{
    ++positionsEvaluated;

    if (depth == 0 || isGameOver(position))
        return staticEvaluation(position);
    
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned nextDepth;
    int eval;

    // if player 1 (high eval is good)
    if (maximizingPlayer == 0)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, &childPosition, i, maximizingPlayer);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax0(&childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);
            if (eval > currentEvaluation)
                currentEvaluation = eval;

            // if P2's current best option (lower is better) is better than this:
            //  P2 would never chose this path so we can prune
            if (beta <= eval)
                break;
            
            // if eval is higher than alpha:
            // we have found a new best move overall!
            // update alpha
            if (eval > alpha)
                alpha = eval;
            
        }
        return currentEvaluation;
    }
    // if player 2 (low eval is good)
    else
    {
        int currentEvaluation = 100;
        for (size_t i = 12; i >= 7; i--)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, &childPosition, i, maximizingPlayer);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax0(&childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);
            if (eval < currentEvaluation)
                currentEvaluation = eval;

            // if P1's current best option (higher is better) is better than this:
            //  P1 would never chose this path so we can prune
            if (eval <= alpha)
                break;

            // if eval is less than beta:
            // we have found a new best position overall!
            // update beta
            if (eval < beta)
                beta = eval;

        }
        return currentEvaluation;
    }
}

unsigned moves[1024];
size_t moveIndex = 0;
size_t numBestMoves = 0;
unsigned bestMovesFirst = 1;

int minimaxBestMoveFirst(Position* position, unsigned depth, int alpha, int beta, unsigned maximizingPlayer)
{
    if (depth == 0 || isGameOver(position))
    {
        bestMovesFirst = 0;
        return staticEvaluation(position);
    }

    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned nextDepth;
    int currentEvaluation;
    int eval;

    if (maximizingPlayer == 0)
    {
        currentEvaluation = -100;
    }
    else
    {
        currentEvaluation = 100;
    }

    if (bestMovesFirst)
    {
        if (moveIndex >= numBestMoves)
        {
            bestMovesFirst = 0;
        }
        else
        {
            unsigned move = moves[moveIndex];
            moveIndex++;
            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, &childPosition, move, maximizingPlayer);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimaxBestMoveFirst(&childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);

            if (maximizingPlayer == 0)
            {
                if (eval > currentEvaluation)
                    currentEvaluation = eval;

                if (beta <= eval)
                    return currentEvaluation;
                
                if (eval > alpha)
                    alpha = eval;
            }
            else
            {
                if (eval < currentEvaluation)
                    currentEvaluation = eval;

                if (eval <= alpha)
                    return currentEvaluation;

                if (eval < beta)
                    beta = eval;
            }
            printf("Best move: %d, alpha: %d, beta: %d\n", move, alpha, beta);
        }
    }


    // if player 1 (high eval is good)
    if (maximizingPlayer == 0)
    {
        currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, &childPosition, i, maximizingPlayer);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax0(&childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);
            if (eval > currentEvaluation)
                currentEvaluation = eval;

            // if P2's current best option (lower is better) is better than this:
            //  P2 would never chose this path so we can prune
            if (beta <= eval)
                break;
            
            // if eval is higher than alpha:
            // we have found a new best move overall!
            // update alpha
            if (eval > alpha)
                alpha = eval;
            
        }
        return currentEvaluation;
    }
    // if player 2 (low eval is good)
    else
    {
        currentEvaluation = 100;
        for (size_t i = 12; i >= 7; i--)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, &childPosition, i, maximizingPlayer);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax0(&childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);
            if (eval < currentEvaluation)
                currentEvaluation = eval;

            // if P1's current best option (higher is better) is better than this:
            //  P1 would never chose this path so we can prune
            if (eval <= alpha)
                break;

            // if eval is less than beta:
            // we have found a new best position overall!
            // update beta
            if (eval < beta)
                beta = eval;

        }
        return currentEvaluation;
    }
}

unsigned analysePosition(Position* position, unsigned player)
{
    Position childPosition;
    unsigned bestMove;
    int bestEvaluation;

        // if player 1 (high eval is good)
    if (player == 0)
    {
        bestEvaluation = -100;
        for (int i = 0; i < 6; i++)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            unsigned playAgain = generateChildPosition(position, &childPosition, i, player);

            int evaluation;
            if (playAgain)
                evaluation = minimax0(&childPosition, initialDepth, -100, 100, player);
            else
                evaluation = minimax0(&childPosition, initialDepth, -100, 100, !player);

            // PrintPosition(&childPosition, evaluation);

            if (evaluation > bestEvaluation)
            {
                bestEvaluation = evaluation;
                bestMove = i;
            }
        }
    }
    // if player 2 (low eval is good)
    else
    {
        bestEvaluation = 100;
        for (size_t i = 7; i < 13; i++)
        {
            if (position->slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            unsigned playAgain = generateChildPosition(position, &childPosition, i, player);

            int evaluation;
            if (playAgain)
                evaluation = minimax0(&childPosition, initialDepth, -100, 100, player);
            else
                evaluation = minimax0(&childPosition, initialDepth, -100, 100, !player);

            // PrintPosition(&childPosition, evaluation);

            if (evaluation < bestEvaluation)
            {
                bestEvaluation = evaluation;
                bestMove = i;
            }
        }
    }

    printf("Best move: %d\n", bestMove);

    return bestMove;
}

int main()
{
    Position position;

    position.slots[P1First] = 4;
    position.slots[P1Second] = 4;
    position.slots[P1Third] = 4;
    position.slots[P1Fourth] = 4;
    position.slots[P1Fifth] = 4;
    position.slots[P1Sixth] = 4;
    position.slots[P1Goal] = 0;

    position.slots[P2First] = 4;
    position.slots[P2Second] = 4;
    position.slots[P2Third] = 4;
    position.slots[P2Fourth] = 4;
    position.slots[P2Fifth] = 4;
    position.slots[P2Sixth] = 4;
    position.slots[P2Goal] = 0;

            for (unsigned i = move + 1; i <= move + numPips; ++i)
        {
            // skip player 1's goal
            unsigned slot = (((i + 7) % (NUM_SLOTS - 1)) + 7) % NUM_SLOTS;
            ++childPosition.slots[slot];
            endSlot = slot;
        }


    unsigned analyse = 0;
    if (analyse)
    {
        unsigned player = 0;
        Position newPosition;

        // make moves
        const size_t numMoves = 0;
        unsigned moves[numMoves];
        for (size_t i = 0; i < numMoves; i++)
        {
            // check move is legal
            if (((player == 0) && (moves[i] < 0 || moves[i] > 5)) ||
                ((player == 1) && (moves[i] < 7 || moves[i] > 12)))
            {
                printf("Illegal move!\n");
                exit(-1);
            }
            unsigned playAgain = generateChildPosition(&position, &newPosition, moves[i], player);

            memcpy(&position, &newPosition, sizeof(position));

            if (!playAgain)
                player = !player;
        }

        printf("Start Position: \n");
        initialDepth = 12;
        int eval = minimax0(&position, initialDepth, -100, 100, player);
        PrintPosition(&position, eval);
        printf("Moves: ");
        for (size_t i = 0; i < numMoves; i++)
        {
            printf("%d, ", moves[i]);
        }
        printf("\n\n");

        // analyse this position
        while (!isGameOver(&position))
        {
            unsigned bestMove = analysePosition(&position, player);
            unsigned playAgain = generateChildPosition(&position, &newPosition, bestMove, player);

            memcpy(&position, &newPosition, sizeof(position));

            if (!playAgain)
                player = !player;
        }
    }
    else
    {
        initialDepth = 0;
        unsigned depth = initialDepth;
        numBestMoves = 13;
        moves[0] = 2;
        moves[1] = 5;
        moves[2] = 10;
        moves[3] = 1;
        moves[4] = 5;
        moves[5] = 0;
        moves[6] = 11;
        moves[7] = 4;
        moves[8] = 11;
        moves[9] = 5;
        moves[10] = 10;
        moves[11] = 3;
        moves[12] = 7;

        while (depth < 50)
        {
            //ori
            clock_t before0 = clock();
            // bestMovesFirst = 1;
            // moveIndex = 0;
            positionsEvaluated = 0;
            positionsGenerated = 0;
            endgamePositions = 0;
            int val0 = minimax0(&position, depth, -100, 100, 0);
            clock_t timeElapsed0 = clock() - before0;
            float sec0 = (float)timeElapsed0 / (float)CLOCKS_PER_SEC;

            fprintf(stdout, "Final evaluation at depth %d: %d (%fs)\n", depth, val0, sec0);
            fprintf(stdout, "Positions evaluated: %d, Positions generated: %d\n", positionsEvaluated, positionsGenerated);
            fprintf(stdout, "Endgame positions: %d (%f%%)\n", endgamePositions, 100.0f * (float)endgamePositions / (float)positionsGenerated);
            fflush(stdout);

            depth++;
        }
    }

    return 0;
}