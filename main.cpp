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

int main()
{
    printf("Hello World\n");

    // Position position;
    // memset(position.slots, 0, sizeof(position.slots));
    // position.slots[0] = 7;
    // position.slots[12] = 1;

    // int player2Evaluation = minimaxNoDepth(position, -100, 100, 1);

    // printf("eval: %d\n", player2Evaluation);
    // PrintPosition(position);

    // return 0;

    unsigned stones = 17;
    unsigned threads = std::thread::hardware_concurrency();

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    clock_t before = clock();
    EndgameTable table;
    table.computeAllEvaluations(stones, threads);

    clock_t timeElapsed = clock() - before;
    float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Time with %d stones and %d threads: %fs (%fs total)\n", stones, threads, elapsed, sec);

    Position test;
    memset(test.slots, 0, sizeof(test.slots));
    test.slots[0] = stones - 1;
    test.slots[12] = 1;
    int evaluation = table.getEvaluation(test, 1);

    printf("Evaluation: %d\n", evaluation);
    PrintPosition(test);

    // Position startPosition;
    // generateStartPosition(startPosition);
    // std::vector<unsigned> firstMoves = {2, 5};
    // makeMoves(startPosition, PLAYER1, firstMoves);

    // unsigned depth = 20;

    // clock_t before = clock();
    // int evaluation = minimax(startPosition, depth, 49, 50, PLAYER2);

    // clock_t timeElapsed = clock() - before;
    // float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

    // printf("Final evaluation at depth %d: %d (%fs)\n", depth, evaluation, sec);


    // std::vector<unsigned> moves = {2, 5, 10, 1, 5, 0, 9, 5, 0, 8, 9};

    // makeMoves(startPosition, PLAYER1, moves);

    // PrintPosition(startPosition);

    //  unsigned depth = 15;
    //  while (depth < 50)
    //  {
    //      unsigned move = analysePosition(startPosition, PLAYER1, depth);
    //      depth++;
    // }

    // evaluations(startPosition, PLAYER1);
    //endgames();

    //endgamesThenEvaluations(7, 10, 10, 15);

    // playGame(PLAYER1, 12);

    // playSelf();




    printf("Hello World\n");

    return 0;
}
