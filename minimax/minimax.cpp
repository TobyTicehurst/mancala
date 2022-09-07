#include "minimax.hpp"
#include "endgames.hpp"
#include <cstdio>

unsigned isGameOver(Position& position)
{
    return (position.slots[PLAYER1_GOAL] > 24 || position.slots[PLAYER2_GOAL] > 24 || 
           (position.slots[PLAYER1_GOAL] == 24 && position.slots[PLAYER2_GOAL] == 24));
}

static int staticEvaluation(Position& position)
{
    if (position.slots[PLAYER1_GOAL] > 24)
        return 50;
    else if (position.slots[PLAYER2_GOAL] > 24)
        return -50;
    else
        return position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
}

unsigned knowns;
unsigned unknowns;
unsigned zeroDepths;

int minimax(Position& position, unsigned depth, int alpha, int beta, unsigned maximizingPlayer)
{
    if (depth == 0)
    {
        zeroDepths++;
        return staticEvaluation(position);
    }
    if (isGameOver(position))
        return staticEvaluation(position);

    unsigned value = lookupEndgamePosition(position, maximizingPlayer);
    if (value != UNKNOWN)
    {
        knowns++;
        if (value == PLAYER1_WIN)
        {
            return 50;
        }
        else if (value == PLAYER2_WIN)
        {
            return -50;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        unknowns++;
    }

    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned nextDepth;
    int eval;

    // if player 1 (high eval is good)
    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;
            
            bool gameOver = false;
            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax(childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);

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
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            eval = minimax(childPosition, nextDepth, alpha, beta, nextMaximizingPlayer);
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

unsigned start = 0;

int minimaxNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    int eval;

    // if player 1 (high eval is good)
    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            // we know maximizingPlayer at this point
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxNoDepth(childPosition, alpha, beta, nextMaximizingPlayer);

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
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            // player may get a second turn in a row (don't change depth)
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            // nextMaximizingPlayer stays the same if playAgain is true and switches if playAgain is false
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);

            if (gameOver)
            {
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            }
            else
                eval = minimaxNoDepth(childPosition, alpha, beta, nextMaximizingPlayer);
            
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

void printStats()
{
    printf("zeroDepths: %d\n", zeroDepths);
}

void resetStats()
{
    knowns = 0;
    unknowns = 0;
    zeroDepths = 0;
}
