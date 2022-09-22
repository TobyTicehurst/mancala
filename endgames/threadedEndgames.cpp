#include "threadedEndgames.hpp"
#include "endgamesTreeTable.hpp"
#include "minimax.hpp"
#include <cstdio>
#include <thread>
#include <cstring>

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

int endgameMinimax(EndgameTable* table, Position& position, int alpha, int beta, unsigned maximizingPlayer)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
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

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = endgameMinimax(table, childPosition, alpha, beta, nextMaximizingPlayer);

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

            if (gameOver)
            {
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            }
            else
                eval = endgameMinimax(table, childPosition, alpha, beta, nextMaximizingPlayer);
            
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

void EndgameCalculator::player1(unsigned stones)
{
    Position position;
    memset(&position, 0, sizeof(position));

    // number of stones still to be placed for each slot
    unsigned remainders[6];

    /* this is a stars and bars problem */

    // must be at least 1 stone for both player 1 and player 2
    for (unsigned player1Stones = 1; player1Stones < stones; player1Stones++)
    {
        unsigned player2Stones = stones - player1Stones;

        /* For each player 1 slot */

        // slot 0
        remainders[0] = player1Stones;
        for (position.slots[0] = 0; position.slots[0] <= remainders[0]; position.slots[0]++)
        {
            // slot 1
            remainders[1] = remainders[0] - position.slots[0];
            for (position.slots[1] = 0; position.slots[1] <= remainders[1]; position.slots[1]++)
            {
                // slot 2
                remainders[2] = remainders[1] - position.slots[1];
                for (position.slots[2] = 0; position.slots[2] <= remainders[2]; position.slots[2]++)
                {
                    // slot 3
                    remainders[3] = remainders[2] - position.slots[2];
                    for (position.slots[3] = 0; position.slots[3] <= remainders[3]; position.slots[3]++)
                    {
                        // slot 4
                        remainders[4] = remainders[3] - position.slots[3];
                        for (position.slots[4] = 0; position.slots[4] <= remainders[4]; position.slots[4]++)
                        {
                            // slot 5
                            position.slots[5] = remainders[4] - position.slots[4];
                            addToQueue(player2Stones, position);
                        }
                    }
                }
            }
        }
    }
}

void EndgameCalculator::player2(unsigned stones, Position& position)
{
    unsigned remainders[5];
    int indices[5];
    // slot 7
    remainders[0] = stones;
    for (indices[0] = remainders[0]; indices[0] >= 0; indices[0]--)
    {
        position.slots[7] = indices[0];
        // slot 8
        remainders[1] = remainders[0] - position.slots[7];
        for (indices[1] = remainders[1]; indices[1] >= 0; indices[1]--)
        {
            position.slots[8] = indices[1];
            // slot 9
            remainders[2] = remainders[1] - position.slots[8];
            for (indices[2] = remainders[2]; indices[2] >= 0; indices[2]--)
            {
                position.slots[9] = indices[2];
                // slot 10
                remainders[3] = remainders[2] - position.slots[9];
                for (indices[3] = remainders[3]; indices[3] >= 0; indices[3]--)
                {
                    position.slots[10] = indices[3];
                    // slot 11
                    remainders[4] = remainders[3] - position.slots[10];
                    for (indices[4] = remainders[4]; indices[4] >= 0; indices[4]--)
                    {
                        position.slots[11] = indices[4];
                        // slot 12
                        position.slots[12] = remainders[4] - position.slots[11];
                        num++;
                        int player1Evaluation = endgameMinimax(table, position, -100, 100, 0);
                        int player2Evaluation = endgameMinimax(table, position, -100, 100, 1);
                        table->setEvaluation(position, player1Evaluation, player2Evaluation);
                    }
                }
            }
        }
    }
}

void EndgameCalculator::addToQueue(unsigned player2Stones, Position& position)
{
    // printf("Remainder: %d\n", player2Stones);
    // PrintPosition(position);
    memcpy(&jobs[head].position, &position, sizeof(position));
    jobs[head].remainingStones = player2Stones;
    head++;
}

// number of ways to arrange 6 of the slots and 1 remainder
static inline unsigned long long numHalfPositions(unsigned n)
{
    // Stars and bars with n Stars (Stones) and 5 bars (6 Slots)
    // (n + 5)!/(n!*5!)

    return 
    ((n + 6ull) * (n + 5ull) * (n + 4ull) * (n + 3ull) * (n + 2ull) * (n + 1ull) / (6 * 5 * 4 * 3 * 2)) - 
    ((n + 5ull) * (n + 4ull) * (n + 3ull) * (n + 2ull) * (n + 1ull) / (5 * 4 * 3 * 2)) - 1;
}

// total number of positions with a given number of stones
static inline unsigned long long numPositions(unsigned long long n)
{
    // Stars and bars with n Stars (Stones) and 11 bars (12 Slots) minus the positions where one side has 0 stones

    // have to use this method to not overflow an unsigned long long ((n + 11)! is massive when n = 48, but (n + 11)!/n! just fits until n = 50)
    return
    ((n + 11ull) * (n + 10ull) * (n + 9ull) * (n + 8ull) * (n + 7ull) * (n + 6ull) * (n + 5ull) * (n + 4ull) * (n + 3ull) * (n + 2ull) * (n + 1ull) /
    (unsigned long long)(11 * 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2)) -
    (unsigned long long)(2 * (n + 5) * (n + 4) * (n + 3) * (n + 2) * (n + 1) / (5 * 4 * 3 * 2));
}

void EndgameCalculator::computeAllEvaluations(unsigned totalStones, unsigned threadCount)
{
    table = new EndgameTreeTable(totalStones);

    // go through every position and assign it a TreeNode
    table->init();

    printf("Analysing...\n");
    printf("Num threads: %d\n", threadCount);
    
    printf("Num stones (%d): 1", totalStones);
    unsigned long long totalPositions = 0;

    for (unsigned i = 2; i <= totalStones; i++)
    {
        jobs.clear();
        head = 0;
        tail = 0;
        sum = 0;
        num = 0;
        numJobs = 0;
        computeEvaluations(i, threadCount);
        table->mMaxStones = i;
        totalPositions += numPositions(i);
    }
    printf("\n");

    //printf("Lookups: %lld\n", (unsigned long long)lookups);
    //printf("totalPositions: %lld\n", totalPositions);
}

void EndgameCalculator::computeEvaluations(unsigned numStones, unsigned threadCount)
{
    // numStones - 1 as each side need at least 1 stone
    numJobs = numHalfPositions(numStones);
    //printf("Num jobs: %lld\n", numJobs);

    printf(", %u", numStones);
    fflush(stdout);

    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    std::string name;
    for (size_t i = 0; i < threadCount; i++)
    {
        name = std::to_string(i);
        threads.emplace_back(&EndgameCalculator::threadLoopTest, this, name);
    }

    jobs.reserve(numJobs);
    player1(numStones);

    for (auto& thread : threads)
    {
        thread.join();
    }

    // printf("Job sum check; %lld/%lld\n", (unsigned long long)sum, (numJobs * (numJobs - 1ull)) / 2ull);

    // unsigned numPos = numPositions(numStones);
    // unsigned total = num;
    // printf("Num positions check: %u/%u\n", numPos, total);
}

void EndgameCalculator::threadLoopTest(std::string name)
{
    //printf("Start: %s\n", name.c_str());
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
    while (true)
    {
        unsigned current = tail++;
        //printf("Current: %d\n", current);

        if (current >= numJobs)
        {
            //printf("Exit: %s\n", name.c_str());
            return;
        }
        
        // wait for job to be assigned
        while(current >= head)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        sum += current;
        if (jobs[current].remainingStones)
            player2(jobs[current].remainingStones, jobs[current].position);
    }
}
