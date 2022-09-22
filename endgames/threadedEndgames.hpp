#pragma once
#include <atomic>
#include <vector>
#include <string>
#include "position.hpp"
#include "endgamesTable.hpp"

struct Job
{
    Position position;
    unsigned remainingStones;
};

class EndgameCalculator
{
public:
    void computeEvaluations(unsigned numStones, unsigned threadCount);
    void computeAllEvaluations(unsigned maxStones, unsigned threadCount);
 
private:
    void addToQueue(unsigned player2Stones, Position& position);

    // TODO rename
    void player1(unsigned stones);
    void player2(unsigned stones, Position& position);
    
    void threadLoopTest(std::string name);

    /*
     * Tail starts as 0 and Head starts as 0
     * After a write happens, atomic head is incremented
     * A read only reserves an index by atomically reading and incrementing tail
     * Data is actually read only when reservedTail < head
     * Thread will exit if reservedTail is equal to max
     */
    std::vector<Job> jobs;

    std::atomic<unsigned> head{0};
    std::atomic<unsigned> tail{0};
    std::atomic<unsigned long long> sum{0};
    std::atomic<unsigned> num{0};
    unsigned long long numJobs;
public:
    //
    EndgameTable* table;
};