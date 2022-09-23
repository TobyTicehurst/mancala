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

// void playSelf(unsigned depth)
// {
//     unsigned currentPlayer = PLAYER1;
//     Position currentPosition;
//     generateStartPosition(currentPosition);

//     printf("Depth %d: ", depth);

//     while (!isGameOver(currentPosition))
//     {
//         unsigned move = analysePosition(currentPosition, currentPlayer, depth);
//         bool gameOver = false;
//         printf("%02d, ", move);
//         Position childPosition;
//         unsigned playAgain = generateChildPosition(currentPosition, childPosition, move, currentPlayer, gameOver);
//         memcpy(&currentPosition, &childPosition, sizeof(currentPosition));
//         if (!playAgain)
//             currentPlayer = !currentPlayer;
//     }
//     printf("(%d)\n", currentPosition.slots[PLAYER1_GOAL] - currentPosition.slots[PLAYER2_GOAL]);
//     fflush(stdout);
// }

// 2 clocks, 1 for wall clock time, other for sum across threads
void timeAndCalculateEndgames(EndgameCalculator& endgameCalculator, unsigned stones, unsigned threads)
{
    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    clock_t before = clock();

    // compute endgames
    endgameCalculator.computeAllEvaluations(stones, threads);

    clock_t timeElapsed = clock() - before;
    float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Time with %d stones and %d threads: %fs (%fs total)\n", stones, threads, elapsed, sec);
}

// void playSelf(EndgameTable* table)
// {
//     Position startPosition;
//     generateStartPosition(startPosition);
//     std::vector<unsigned> firstMoves = {2, 5};
//     makeMoves(startPosition, PLAYER1, firstMoves);

//     clock_t before = clock();
//     MemoryNode root;
//     int evaluation = mtdfEndgameTableNoDepth(startPosition, 8, PLAYER2, table, &root, 10);

//     clock_t timeElapsed = clock() - before;
//     float sec = (float)timeElapsed / (float)CLOCKS_PER_SEC;

//     printf("Evaluation: %d (%fs)\n", evaluation, sec);

//     exploreTree(&root);
// }

class OppeningTableNode
{
public:
    OppeningTableNode(unsigned m) :
        move{m},
        evaluation{255}
    {}
    OppeningTableNode() :
        move{255},
        evaluation{255}
    {}
    unsigned move;
    int evaluation;
    unsigned size;

    // there will be 1 or 6 of these
    OppeningTableNode* responses;
};

// // for player 2, add all moves to tree
// void addMoves(Position& position, OppeningTableNode* node, EndgameTable* table)
// {
//     Position childPosition;
//     unsigned playAgain;
//     unsigned nextMaximizingPlayer;

//     for (unsigned i = 7; i < 13; i++)
//     {
//         if (position.slots[i] == 0)
//             continue;
        
//         bool gameOver = false;
//         playAgain = generateChildPosition(position, childPosition, i, PLAYER2, gameOver);

//         int evaluation;

//         if (gameOver)
//             evaluation = position.slots[6] - position.slots[13];
//         else if (playAgain)
//             addMoves();
//         else
//             makeBestMove(childPosition, &node->responses[i - 7], table);
//     }
// }

// for player 1 find the best move
int makeBestMove(Position& position, OppeningTableNode* node, EndgameTable* table)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned bestMove;

    int maxEval = -100;
    for (unsigned i = 0; i < 6; i++)
    {
        if (position.slots[i] == 0)
            continue;

        bool gameOver = false;
        playAgain = generateChildPosition(position, childPosition, i, PLAYER1, gameOver);
        nextMaximizingPlayer = !playAgain;

        int evaluation;

        if (gameOver)
            evaluation = position.slots[6] - position.slots[13];
        else
            evaluation = mtdfNoMemoryEndgameTableNoDepth(childPosition, node->evaluation, nextMaximizingPlayer, table);

        if (evaluation > maxEval)
        {
            maxEval = evaluation;
            bestMove = i;
        }
    }

    node->evaluation = maxEval;
    node->move = bestMove;
}

void printOpeningTable(OppeningTableNode* node, unsigned depth)
{
    if (node == nullptr)
        return;

    for (unsigned i = 0; i < depth; i++)
    {
        printf("\t");
    }
    printf("{\n");
    printf("\tMove: %d, Evaluation: %d\n", node->move, node->evaluation);

    for (unsigned i = 0; i < node->size; i++)
    {
        if (node && node->responses)
            printOpeningTable(&node->responses[i], depth + 1);
    }
    printf("}\n");
}

int getBestMove(Position& position, EndgameTable* table, int evaluationGuess, int& evaluation)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    unsigned bestMove;

    int maxEval = -100;
    for (unsigned i = 0; i < 6; i++)
    {
        if (position.slots[i] == 0)
            continue;

        bool gameOver = false;
        playAgain = generateChildPosition(position, childPosition, i, PLAYER1, gameOver);

        printf("Exploring position:\n");
        PrintPosition(childPosition);

        nextMaximizingPlayer = !playAgain;

        int evaluation;

        if (gameOver)
            evaluation = position.slots[6] - position.slots[13];
        else
            evaluation = mtdfNoMemoryEndgameTableNoDepth(childPosition, evaluationGuess, nextMaximizingPlayer, table);

        if (evaluation > maxEval)
        {
            maxEval = evaluation;
            bestMove = i;
        }
    }

    evaluation = maxEval;
    return bestMove;
}

void addToOpeningTable(OppeningTableNode* node, Position& position, unsigned cpuPlayer, unsigned currentPlayer, int depth, EndgameTable* endgameTable)
{
    if (depth == 0)
        return;
    
    printf("Opening table, depth: %d\n", depth);

    Position childPosition;

    if (currentPlayer == cpuPlayer)
    {
        node->move = getBestMove(position, endgameTable, 8, node->evaluation);

        bool gameOver;
        bool playAgain = generateChildPosition(position, childPosition, node->move, currentPlayer, gameOver);
        if (gameOver)
            return;
        
        if (playAgain)
        {
            node->responses = new OppeningTableNode;
            node->size = 1;
            node = node->responses;
            addToOpeningTable(node, childPosition, cpuPlayer, currentPlayer, depth, endgameTable);
        }
        else
        {
            currentPlayer = !currentPlayer;
            addToOpeningTable(node, childPosition, cpuPlayer, currentPlayer, depth - 1, endgameTable);
        }
    }
    else
    {
        node->responses = new OppeningTableNode[6];
        node->size = 6;
        
        for (unsigned i = 0; i < 6; i++)
        {
            if (position.slots[i] == 0)
                continue;
            
            bool gameOver;
            bool playAgain = generateChildPosition(position, childPosition, i, currentPlayer, gameOver);

            if (gameOver)
                continue;

            if (playAgain)
            {
                addToOpeningTable(&node->responses[i], childPosition, cpuPlayer, currentPlayer, depth, endgameTable);
            }
            else
            {
                currentPlayer = !currentPlayer;
                addToOpeningTable(&node->responses[i], childPosition, cpuPlayer, currentPlayer, depth - 1, endgameTable);
            }
        }
    }
}

// void createOpeningTable(OppeningTableNode& openingTable, unsigned cpuPlayer, int maxDepth, EndgameTable* endgameTable)
// {
//     Position position;
//     Position childPosition;
//     unsigned currentPlayer = PLAYER1;
//     generateStartPosition(position);

//     OppeningTableNode* node = &openingTable;

//     int depth = 0;
//     while (depth <= maxDepth)
//     {
//         if (currentPlayer == cpuPlayer)
//         {
//             openingTable.move = getBestMove(position, endgameTable, 8, node->evaluation);

//             bool gameOver;
//             bool playAgain = generateChildPosition(position, childPosition, openingTable.move, currentPlayer, gameOver);
//             if (playAgain)
//             {
//                 node->responses = new OppeningTableNode(255);
//                 node = node->responses;
//             }
//         }
//         else
//         {
//             node->responses = new OppeningTableNode[6];
//             // need to recurse here
//         }
//     }

// }

// void playerUser(OppeningTableNode* openingTable, EndgameTable* endgameTable)
// {
//     OppeningTableNode* node = openingTable;
//     unsigned cpuPlayer = PLAYER1;
//     unsigned currentPlayer = PLAYER1;
//     Position position;
//     Position childPosition;
//     bool gameOver;
//     bool playAgain;
//     unsigned move;
//     int evaluation;

//     while (true)
//     {
//         PrintPosition(position);
        
//         if (currentPlayer == cpuPlayer)
//         {
//             if (node)
//             {
//                 move = node->move;
//                 evaluation = node->evaluation;
//             }
//             else
//             {
//                 int newEvaluation;
//                 move = getBestMove(position, endgameTable, evaluation, newEvaluation);
//                 evaluation = newEvaluation;
//             }
            
//             printf("Evaluation: %d\n", evaluation);
//             fflush(stdout);
            
//             playAgain = generateChildPosition(position, childPosition, node->move, currentPlayer, gameOver);
//             if (playAgain && node)
//                 node = node->responses;
//         }
//         else
//         {
//             // gets move between 0 and 5
//             move = getUserMove();
//             playAgain = generateChildPosition(position, childPosition, move + currentPlayer * 7, currentPlayer, gameOver);

//             if (node)
//             {
//                 if (node->responses)
//                     node = &node->responses[move];
//                 else
//                     node = nullptr;
//             }
//         }

//         memcpy(&position, &childPosition, sizeof(position));
//         if (gameOver)
//             break;
//         if (!playAgain)
//             currentPlayer = !currentPlayer;
//     }
// }

int main()
{
    printf("Hello World\n");



    // std::vector<unsigned> firstMoves = {2, 5, 4 + 7, 0, 3 + 7, 0, 4 + 7, 1, 2 + 7, 0, 4 + 7, 1};
    // makeMoves(startPosition, PLAYER1, firstMoves);

    EndgameCalculator endgameCalculator;
    unsigned stones = 26;
    unsigned threads = std::thread::hardware_concurrency();
    timeAndCalculateEndgames(endgameCalculator, stones, threads);

    Position startPosition;
    generateStartPosition(startPosition);

    OppeningTableNode openingTable;
    addToOpeningTable(&openingTable, startPosition, PLAYER1, PLAYER1, 1, endgameCalculator.table);

    printOpeningTable(&openingTable, 0);

    // playSelf(endgameCalculator.table);

    printf("Hello World\n");

    return 0;
}
