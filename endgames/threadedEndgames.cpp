#include "threadedEndgames.hpp"
#include "minimax.hpp"
#include <cstdio>
#include <thread>
#include <cstring>

#define BITS_FOR_PLAYER 1
#define BITS_FOR_STONES 12

// void EndgameTable::reserve(unsigned maxStones)
// {
//     this->maxStones = maxStones;
//     unsigned power = BITS_FOR_PLAYER + BITS_FOR_STONES + maxStones;
//     if (power > 31)
//     {
//         printf("Error: Overflowing an unsigned\n");
//         exit(-1);
//     }

//     evaluations.clear();
//     evaluations.reserve(1 << power);
// }

// void EndgameTable::evaluate(unsigned maxStones)
// {
//     reserve(maxStones);

//     // threadsafe evals
//     unsigned numThreads = 2;

//     // evaluate all 2 stone positions first. Can then look these up

//     for (unsigned numStones = 0; numStones <= maxStones; numStones++)
//     {
//         evaluateAll(numStones);

//         // only allow lookups once we know there won't be any more writes
//         this->maxStones = numStones;
//     }
// }

// void EndgameTable::evaluateAll(unsigned numStones)
// {
//     Position position;
//     for (unsigned player1Stones = 1; player1Stones < stonesInPlay; player1Stones++)
//     {
//         unsigned player2Stones = stonesInPlay - player1Stones;
//         unsigned player1Remainder = player1Stones;

//         unsigned stonesInGoals = 48 - stonesInPlay;
//         for (unsigned stonesInGoal1 = 24 - stonesInPlay; stonesInGoal1 <= 24; stonesInGoal1++)
//         {
//             unsigned stonesInGoal2 = 48 - stonesInPlay - stonesInGoal1;
//             position.slots[PLAYER1_GOAL] = stonesInGoal1;
//             position.slots[PLAYER2_GOAL] = stonesInGoal2;
//             generatePlayer1Slots(position, player1Stones, 0, player2Stones, stonesInPlay);
//         }
//     }
// }

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

int EndgameTable::endgameMinimax(Position& position, int alpha, int beta, unsigned maximizingPlayer)
{
    Position childPosition;
    unsigned playAgain;
    unsigned nextMaximizingPlayer;
    int eval;

    unsigned remainingStones = getRemainingStones(position);
    if (remainingStones <= maxStones)
    {
        lookups++;
        return getEvaluation(position, maximizingPlayer) + position.slots[PLAYER1_GOAL] - position.slots[PLAYER2_GOAL];
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
            nextMaximizingPlayer = (playAgain && maximizingPlayer) || (!playAgain && !maximizingPlayer);

            if (gameOver)
                eval = childPosition.slots[PLAYER1_GOAL] - childPosition.slots[PLAYER2_GOAL];
            else
                eval = endgameMinimax(childPosition, alpha, beta, nextMaximizingPlayer);

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
                eval = endgameMinimax(childPosition, alpha, beta, nextMaximizingPlayer);
            
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

void EndgameTable::player1(unsigned stones)
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

static void doubleIndexFromPosition(Position& position, unsigned& player1Index, unsigned& player2Index)
{
    player1Index = 2;

    player1Index <<= position.slots[12];
    player1Index += 1;

    // Playing slots (not goals)
    for (size_t i = 11; i > 6; i--)
    {
        player1Index <<= position.slots[i] + 1;
        player1Index += 1;
    }
    for (size_t i = 5; i > 0; i--)
    {
        player1Index <<= position.slots[i] + 1;
        player1Index += 1;
    }
    player1Index <<= position.slots[0];

    player1Index <<= 1;

    player2Index = player1Index;
    player2Index |= 0x1;
}

static unsigned indexFromPosition(Position& position, unsigned player)
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

    // whos turn is it?
    index <<= 1;
    index |= player;

    return index;
}

void EndgameTable::player2(unsigned stones, Position& position)
{
    unsigned remainders[6];
    // slot 7
    remainders[0] = stones;
    for (position.slots[7] = 0; position.slots[7] <= remainders[0]; position.slots[7]++)
    {
        // slot 8
        remainders[1] = remainders[0] - position.slots[7];
        for (position.slots[8] = 0; position.slots[8] <= remainders[1]; position.slots[8]++)
        {
            // slot 9
            remainders[2] = remainders[1] - position.slots[8];
            for (position.slots[9] = 0; position.slots[9] <= remainders[2]; position.slots[9]++)
            {
                // slot 10
                remainders[3] = remainders[2] - position.slots[9];
                for (position.slots[10] = 0; position.slots[10] <= remainders[3]; position.slots[10]++)
                {
                    // slot 11
                    remainders[4] = remainders[3] - position.slots[10];
                    for (position.slots[11] = 0; position.slots[11] <= remainders[4]; position.slots[11]++)
                    {
                        // slot 12
                        position.slots[12] = remainders[4] - position.slots[11];
                        num++;
                        int player1Evaluation = endgameMinimax(position, -100, 100, 0);
                        int player2Evaluation = endgameMinimax(position, -100, 100, 1);
                        setEvaluation(position, 0, player1Evaluation);
                        setEvaluation(position, 1, player2Evaluation);

                        bool print = false;//(position.slots[0] == 7 && position.slots[12] == 1);

                        if (print)
                        {
                            printf("Eval: %d, %d\n", player1Evaluation, player2Evaluation);
                            PrintPosition(position);
                        }
                    }
                }
            }
        }
    }
}

void EndgameTable::addToQueue(unsigned player2Stones, Position& position)
{
    // printf("Remainder: %d\n", player2Stones);
    // PrintPosition(position);
    memcpy(&jobs[head].position, &position, sizeof(position));
    jobs[head].remainingStones = player2Stones;
    head++;
    if (head > numJobs)
        printf("Error, too many jobs: %d/%lld\n", (unsigned)head, numJobs);
}

// static inline unsigned numHalfPositions(unsigned n)
// {
//     // Stars and bars with n Stars (Stones) and 5 bars (6 Slots)
//     // (n + 5)!/(n!*5!)
//     return (n + 5) * (n + 4) * (n + 3) * (n + 2) * (n + 1) / 120;
// }

static inline unsigned fact(unsigned x)
{
    unsigned ret = 1;
    for (unsigned i = 2; i <= x; i++)
    {
        ret *= i;
    }
    return ret;
}

static inline unsigned starsAndBars(unsigned slots, unsigned stones)
{
    return fact(slots + stones - 1) / (fact(stones) * fact(slots - 1));
}

// number of ways to arrange 6 of the slots and 1 remainder
static inline unsigned long long numHalfPositions(unsigned n)
{
    // Stars and bars with n Stars (Stones) and 5 bars (6 Slots)
    // (n + 5)!/(n!*5!)
    //return (fact(n + 6) / (fact(n) * fact(6))) - (fact(n + 5) / (fact(n) * fact(5))) - 1;

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

void EndgameTable::computeAllEvaluations(unsigned totalStones, unsigned threadCount)
{
    unsigned long long totalPositions = 0;
    unsigned power = BITS_FOR_PLAYER + BITS_FOR_STONES + totalStones;
    if (power > 31)
    {
        printf("Error: Overflowing an unsigned\n");
        exit(-1);
    }

    evaluations.clear();
    evaluations.reserve(1 << power);

    for (unsigned i = 2; i <= totalStones; i++)
    {
        jobs.clear();
        head = 0;
        tail = 0;
        sum = 0;
        num = 0;
        numJobs = 0;
        computeEvaluations(i, threadCount);
        maxStones = i;
        totalPositions += numPositions(i);
    }

    printf("Lookups: %d\n", (unsigned)lookups);
    printf("totalPositions: %lld\n", totalPositions);
}

void EndgameTable::computeEvaluations(unsigned numStones, unsigned threadCount)
{
    // printf("test1 = %d\n", numHalfPositions(3));
    // printf("test2 = %d\n", starsAndBars(7, 3));
    // printf("test3 = %d\n", starsAndBars(6, 3));

    printf("Num threads: %d\n", threadCount);

    // numStones - 1 as each side need at least 1 stone
    numJobs = numHalfPositions(numStones);
    printf("Num jobs: %lld\n", numJobs);

    printf("Stones: %u\n", numStones);

    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    std::string name;
    for (size_t i = 0; i < threadCount; i++)
    {
        name = std::to_string(i);
        threads.emplace_back(&EndgameTable::threadLoopTest, this, name);
    }

    jobs.reserve(numJobs);
    player1(numStones);

    for (auto& thread : threads)
    {
        thread.join();
    }

    printf("Job sum check; %lld/%lld\n", (unsigned long long)sum, (numJobs * (numJobs - 1ull)) / 2ull);

    unsigned numPos = numPositions(numStones);
    unsigned total = num;
    printf("Num positions check: %u/%u\n", numPos, total);
}

void EndgameTable::threadLoopTest(std::string name)
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

unsigned EndgameTable::getMaxStones()
{
    return maxStones;
}

int EndgameTable::setEvaluation(Position& position, bool player, int evaluation)
{
    unsigned index = indexFromPosition(position, player);
    evaluations[index] = evaluation;
}

int EndgameTable::getEvaluation(Position& position, bool player)
{
    unsigned index = indexFromPosition(position, player);
    return evaluations[index];
}

struct TreeNode
{
    union
    {
        TreeNode* children[12];
        int* evaluations[12];
    };
};

// assumes tree is not null
int EndgameTable::setEvaluation2(Position& position, bool player, int evaluation)
{
    TreeNode* node = treeArray[
        position.slots[0] +
        position.slots[1] * maxStones +
        position.slots[2] * maxStones * maxStones +
        position.slots[3] * maxStones * maxStones * maxStones +
        position.slots[4] * maxStones * maxStones * maxStones * maxStones +
        position.slots[5] * maxStones * maxStones * maxStones * maxStones * maxStones
    ];

    for (size_t i = 7; i < 12; i++)
    {
        node = node->children[position.slots[i]];
        if (node == NULL)
        {
            node = new TreeNode;
            memset(node, 0, sizeof(*node));
        }
    }

    if (node == NULL)
    {
        node = new TreeNode;
    }

    int* evalPtr = node->evaluations[position.slots[12]];
    evalPtr = new int(evaluation);
}