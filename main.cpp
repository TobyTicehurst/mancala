#include "endgames.hpp"
#include "position.hpp"
#include "minimax.hpp"
#include <cstdio>
#include <cstring>
#include <time.h>
#include <iostream>
#include <vector>
#include <thread>
#include "threadedEndgames.hpp"


void endgames()
{
    unsigned numStones = 2;
    while (numStones <= 12)
    {
        clock_t before = clock();
        generateAllPositions(numStones);

        clock_t timeElapsed = clock() - before;
        float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

        printf("Endgames with %d stones: (%fs)\n", numStones, sec);
        numStones++;
    }
}

void evaluations(Position& startPosition, unsigned player)
{
    unsigned depth = 0;
    while (depth < 50)
    {
        resetPositionStats();
        clock_t before = clock();
        int evaluation = minimax(startPosition, depth, -100, 100, player);
        clock_t timeElapsed = clock() - before;
        float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

        printf("Final evaluation at depth %d: %d (%fs)\n", depth, evaluation, sec);
        printPositionStats();
        depth++;
    }
}

void evaluations()
{
    Position startPosition;
    generateStartPosition(startPosition);

    evaluations(startPosition, PLAYER1);
}

void endgamesThenEvaluations(unsigned endgameStonesMin, unsigned endgameStonesMax, unsigned depthMin, unsigned depthMax)
{
    unsigned numStones = endgameStonesMin;
    while (numStones <= endgameStonesMax)
    {
        resetStats();
        clock_t before = clock();
        generateAllPositions(numStones);

        clock_t timeElapsed = clock() - before;
        float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

        printf("Endgames with %d stones: (%fs)\n", numStones, sec);

        printStats();

        Position startPosition;
        generateStartPosition(startPosition);

        unsigned depth = depthMin;
        while (depth < depthMax)
        {
            resetStats();
            clock_t beforeEval = clock();
            int evaluation = minimax(startPosition, depth, -100, 100, 0);
            clock_t timeElapsedEval = clock() - beforeEval;
            float secEval = (float)timeElapsedEval / (float)CLOCKS_PER_SEC;

            printf("Final evaluation at depth %d: %d (%fs)\n", depth, evaluation, secEval);

            printStats();

            depth++;
        }

        numStones++;
    }
}

void makeMoves(Position& position, unsigned startPlayer, std::vector<unsigned>& moves)
{
    Position childPosition;
    unsigned player = startPlayer;
    for (size_t i = 0; i < moves.size(); i++)
    {
        bool gameOver = false;
        unsigned playAgain = generateChildPosition(position, childPosition, moves[i], player, gameOver);
        if (!playAgain)
            player = !player;
        memcpy(&position, &childPosition, sizeof(position));
        PrintPosition(position);
    }
}

unsigned analysePosition(Position position, unsigned player, unsigned depth)
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
            if (position.slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            bool gameOver = false;
            unsigned playAgain = generateChildPosition(position, childPosition, i, player, gameOver);

            int evaluation;
            if (playAgain)
                evaluation = minimax(childPosition, depth, -100, 100, player);
            else
                evaluation = minimax(childPosition, depth, -100, 100, !player);

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
            if (position.slots[i] == 0)
                continue;

            // player may get a second turn in a row (don't change depth)
            bool gameOver = false;
            unsigned playAgain = generateChildPosition(position, childPosition, i, player, gameOver);

            int evaluation;
            if (playAgain)
                evaluation = minimax(childPosition, depth, -100, 100, player);
            else
                evaluation = minimax(childPosition, depth, -100, 100, !player);

            // PrintPosition(&childPosition, evaluation);

            if (evaluation < bestEvaluation)
            {
                bestEvaluation = evaluation;
                bestMove = i;
            }
        }
    }

    printf("Best move: %d, Eval: %d, Depth: %d\n", bestMove, bestEvaluation, depth);

    return bestMove;
}

void playGame(unsigned humanPlayer, unsigned depth)
{
    Position currentPosition;
    generateStartPosition(currentPosition);
    PrintPosition(currentPosition);

    unsigned currentPlayer = PLAYER1;
    unsigned cpuPlayer = !humanPlayer;
    unsigned move;

    while (true)
    {
        if (currentPlayer == cpuPlayer)
        {
            printf("Thinking... \n");
            move = analysePosition(currentPosition, currentPlayer, depth);
        }
        else
        {
            bool validMove = false;
            do
            {
                printf("Enter a move: ");
                std::cin >> move;
                
                if ((humanPlayer == PLAYER1 && move < 6) ||
                    (humanPlayer == PLAYER2 && move > 6 && move < 13) &&
                    currentPosition.slots[move] != 0)
                {
                    validMove = true;
                }
            } while (!validMove);
            printf("\n");
        }

        Position childPosition;
        bool gameOver = false;
        unsigned playAgain = generateChildPosition(currentPosition, childPosition, move, currentPlayer, gameOver);
        memcpy(&currentPosition, &childPosition, sizeof(currentPosition));
        PrintPosition(currentPosition);
        if (!playAgain)
        {
            currentPlayer = !currentPlayer;
        }
        else
        {
            printf("Play again!\n");
        }
    }
}

void playSelf(unsigned depth)
{
    unsigned currentPlayer = PLAYER1;
    Position currentPosition;
    generateStartPosition(currentPosition);

    printf("Depth %d: ", depth);

    while (!isGameOver(currentPosition))
    {
        unsigned move = analysePosition(currentPosition, currentPlayer, depth);
        bool gameOver = false;
        printf("%02d, ", move);
        Position childPosition;
        unsigned playAgain = generateChildPosition(currentPosition, childPosition, move, currentPlayer, gameOver);
        memcpy(&currentPosition, &childPosition, sizeof(currentPosition));
        if (!playAgain)
            currentPlayer = !currentPlayer;
    }
    printf("(%d)\n", currentPosition.slots[PLAYER1_GOAL] - currentPosition.slots[PLAYER2_GOAL]);
    fflush(stdout);
}

void playSelf()
{
    unsigned depth = 0;
    while (depth < 50)
    {
        playSelf(depth);
        depth++;
    }
}

int mtdfNoMemory(Position& position, int initialGuess, unsigned depth) {
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do {
        // if we have the lower bound, get the upper bound
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = minimax(position, depth, beta - 1, beta, PLAYER1);

        // currentEvaluation will be the lower bound if the true value is lower than currentEvaluation
        // currentEvaluation will be the upper bound if the true value is higher than currentEvaluation
        // set the corresponding bound
        bound[currentEvaluation < beta] = currentEvaluation;

    } while (bound[0] < bound[1]); // when bounds are equal we have the result

    return currentEvaluation;
}

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

int endgameMinimaxDepth(EndgameTable* table, Position& position, int alpha, int beta, unsigned maximizingPlayer, unsigned depth)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned nextDepth;
    int eval;

    unsigned remainingStones = getRemainingStones(position);
    if (remainingStones <= table->mMaxStones)
    {
        return table->getEvaluation(position, maximizingPlayer) + position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
    }

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

            if (gameOver || (nextDepth == 0))
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = endgameMinimaxDepth(table, childPosition, alpha, beta, nextMaximizingPlayer, nextDepth);

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

            if (gameOver || (nextDepth == 0))
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = endgameMinimaxDepth(table, childPosition, alpha, beta, nextMaximizingPlayer, nextDepth);
            
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

int mtdfNoMemoryEndgames(EndgameTable* table, Position& position, unsigned player, int initialGuess, unsigned depth) {
    int bound[2] = {-100, +100}; // lower, upper
    int beta;
    int currentEvaluation = initialGuess;
    do {
        // if we have the lower bound, get the upper bound
        beta = currentEvaluation + (currentEvaluation == bound[0]);
        currentEvaluation = endgameMinimaxDepth(table, position, beta - 1, beta, player, depth);
        // printf("currentEvaluation: %d, beta: %d\n", currentEvaluation, beta);

        // currentEvaluation will be the lower bound if the true value is lower than currentEvaluation
        // currentEvaluation will be the upper bound if the true value is higher than currentEvaluation
        // set the corresponding bound
        bound[currentEvaluation < beta] = currentEvaluation;

    } while (bound[0] < bound[1]); // when bounds are equal we have the result

    return currentEvaluation;
}

int main()
{
    printf("Hello World\n");

    unsigned stones = 24;
    unsigned threads = std::thread::hardware_concurrency();

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    clock_t before = clock();
    EndgameCalculator endgameCalculator;
    endgameCalculator.computeAllEvaluations(stones, threads);

    clock_t timeElapsed = clock() - before;
    float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Time with %d stones and %d threads: %fs (%fs total)\n", stones, threads, elapsed, sec);

    Position startPosition;
    generateStartPosition(startPosition);
    std::vector<unsigned> firstMoves = {2, 5};
    makeMoves(startPosition, PLAYER1, firstMoves);

    unsigned depth = 1;
    while (depth < 50)
    {
        clock_t before = clock();
        int evaluation = mtdfNoMemoryEndgames(endgameCalculator.table, startPosition, PLAYER2, 8, depth);

        clock_t timeElapsed = clock() - before;
        float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

        printf("Final evaluation at depth %d: %d (%fs)\n", depth, evaluation, sec);
        depth++;
    }

    printf("Hello World\n");

    return 0;
}
