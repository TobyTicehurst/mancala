#include "minimax.hpp"
#include <cstring>

// TODO put in position
static inline unsigned getRemainingStones(const Position& position)
{
    return position.slots[0] + 
           position.slots[1] + 
           position.slots[2] + 
           position.slots[3] + 
           position.slots[4] + 
           position.slots[5] +
           position.slots[7] + 
           position.slots[8] + 
           position.slots[9] + 
           position.slots[10] + 
           position.slots[11] + 
           position.slots[12];
}

/* minimax functions */

int minimax(Position& position, int alpha, int beta, unsigned maximizingPlayer, unsigned depth)
{
    if (depth == 0)
        return position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    
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
            // we know maximizingPlayer at this point
            nextMaximizingPlayer = !playAgain;
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimax(childPosition, alpha, beta, nextMaximizingPlayer, nextDepth);

            // PrintPosition(childPosition);
            // printf("Eval: %d\n", eval);

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
            nextMaximizingPlayer = playAgain;
            // next Depth reduces by 1 if playAgain is false
            nextDepth = depth - !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimax(childPosition, alpha, beta, nextMaximizingPlayer, nextDepth);
            
            // PrintPosition(childPosition);
            // printf("Eval: %d\n", eval);

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

int minimaxNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    int eval;

    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxNoDepth(childPosition, alpha, beta, nextMaximizingPlayer);

            if (eval > currentEvaluation)
                currentEvaluation = eval;
            if (beta <= eval)
                break;
            if (eval > alpha)
                alpha = eval;
        }
        return currentEvaluation;
    }
    else
    {
        int currentEvaluation = 100;
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxNoDepth(childPosition, alpha, beta, nextMaximizingPlayer);

            if (eval < currentEvaluation)
                currentEvaluation = eval;
            if (eval <= alpha)
                break;
            if (eval < beta)
                beta = eval;
        }
        return currentEvaluation;
    }
}

int minimaxEndgameTable(Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table, unsigned depth)
{
    if (depth == 0)
        return position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];

    if (getRemainingStones(position) <= table->mMaxStones)
        return table->getEvaluation(position, maximizingPlayer) + position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned nextDepth;
    int eval;

    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = !playAgain;
            nextDepth = depth - !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxEndgameTable(childPosition, alpha, beta, nextMaximizingPlayer, table, nextDepth);

            if (eval > currentEvaluation)
                currentEvaluation = eval;
            if (beta <= eval)
                break;
            if (eval > alpha)
                alpha = eval;
        }
        return currentEvaluation;
    }
    else
    {
        int currentEvaluation = 100;
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = playAgain;
            nextDepth = depth - !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxEndgameTable(childPosition, alpha, beta, nextMaximizingPlayer, table, nextDepth);

            if (eval < currentEvaluation)
                currentEvaluation = eval;
            if (eval <= alpha)
                break;
            if (eval < beta)
                beta = eval;
        }
        return currentEvaluation;
    }
}

int minimaxEndgameTableNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table)
{
    if (getRemainingStones(position) <= table->mMaxStones)
        return table->getEvaluation(position, maximizingPlayer) + position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    int eval;

    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table);

            if (eval > currentEvaluation)
                currentEvaluation = eval;
            if (beta <= eval)
                break;
            if (eval > alpha)
                alpha = eval;
        }
        return currentEvaluation;
    }
    else
    {
        int currentEvaluation = 100;
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = minimaxEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table);

            if (eval < currentEvaluation)
                currentEvaluation = eval;
            if (eval <= alpha)
                break;
            if (eval < beta)
                beta = eval;
        }
        return currentEvaluation;
    }
}

int minimaxMemoryEndgameTableNoDepth(Position& position, int alpha, int beta, unsigned maximizingPlayer, EndgameTable* table, MemoryNode* node, int depth)
{
    if (getRemainingStones(position) <= table->mMaxStones)
        return table->getEvaluation(position, maximizingPlayer) + position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    
    if (depth > 0)
    {
        node->children = new MemoryNode[6];
        memset(node->children, 0, sizeof(node->children[0]) * 6);
    }

    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    int eval;

    if (maximizingPlayer == PLAYER1)
    {
        int currentEvaluation = -100;
        for (int i = 5; i >= 0; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = !playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else if (depth > 0)
            {
                eval = minimaxMemoryEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table, &node->children[i], depth - 1);
                node->children[i].evaluation = eval;
            }
            else
            {
                eval = minimaxEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table);
            }

            if (eval > currentEvaluation)
                currentEvaluation = eval;
            if (beta <= eval)
                break;
            if (eval > alpha)
                alpha = eval;
        }
        return currentEvaluation;
    }
    else
    {
        int currentEvaluation = 100;
        for (unsigned i = 12; i >= 7; i--)
        {
            if (position.slots[i] == 0)
                continue;

            bool gameOver = false;
            playAgain = generateChildPosition(position, childPosition, i, maximizingPlayer, gameOver);
            nextMaximizingPlayer = playAgain;

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else if (depth > 0)
            {
                eval = minimaxMemoryEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table, &node->children[i - 7], depth - 1);
                node->children[i - 7].evaluation = eval;
            }
            else
            {
                eval = minimaxEndgameTableNoDepth(childPosition, alpha, beta, nextMaximizingPlayer, table);
            }

            if (eval < currentEvaluation)
                currentEvaluation = eval;
            if (eval <= alpha)
                break;
            if (eval < beta)
                beta = eval;
        }
        return currentEvaluation;
    }
}

/* MTD(F) functions */

int mtdfNoMemory(Position& position, int initialGuess, unsigned maximizingPlayer, unsigned depth)
{
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do
    {
        // if we have the lower bound, get the upper bound
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimax(position, depth, beta - 1, beta, maximizingPlayer);

        // currentEvaluation will be the lower bound if the true value is lower than currentEvaluation
        // currentEvaluation will be the upper bound if the true value is higher than currentEvaluation
        // set the corresponding bound
        bound[currentEvaluation < beta] = currentEvaluation;
    }
    while (bound[0] < bound[1]); // when bounds are equal we have the result

    return currentEvaluation;
}

int mtdfNoMemoryNoDepth(Position& position, int initialGuess, unsigned maximizingPlayer)
{
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do
    {
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimaxNoDepth(position, beta - 1, beta, maximizingPlayer);
        bound[currentEvaluation < beta] = currentEvaluation;
    }
    while (bound[0] < bound[1]);

    return currentEvaluation;
}

int mtdfNoMemoryEndgameTable(Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table, unsigned depth)
{
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do
    {
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimaxEndgameTable(position, beta - 1, beta, maximizingPlayer, table, depth);
        bound[currentEvaluation < beta] = currentEvaluation;
    }
    while (bound[0] < bound[1]);

    return currentEvaluation;
}

int mtdfNoMemoryEndgameTableNoDepth(Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table)
{
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do
    {
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimaxEndgameTableNoDepth(position, beta - 1, beta, maximizingPlayer, table);
        bound[currentEvaluation < beta] = currentEvaluation;
    }
    while (bound[0] < bound[1]);

    return currentEvaluation;
}

int mtdfEndgameTableNoDepth(Position& position, int initialGuess, unsigned maximizingPlayer, EndgameTable* table, MemoryNode* root, int depth)
{
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do
    {
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimaxMemoryEndgameTableNoDepth(position, beta - 1, beta, maximizingPlayer, table, root, depth);
        bound[currentEvaluation < beta] = currentEvaluation;
    }
    while (bound[0] < bound[1]);

    return currentEvaluation;
}

