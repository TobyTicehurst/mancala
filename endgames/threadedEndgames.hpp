#pragma once
#include <atomic>
#include <vector>
#include <string>
#include "position.hpp"

struct Job
{
    Position position;
    unsigned remainingStones;
};

class EndgameTable
{
public:
    // void evaluate(unsigned maxStones);
    // void lookup(Position& position);
    
    void computeEvaluations(unsigned numStones, unsigned threadCount);
    void computeAllEvaluations(unsigned maxStones, unsigned threadCount);
    void threadLoopTest(std::string name);
    void addToQueue(unsigned player2Stones, Position& position);
    void player1(unsigned stones);
    void player2(unsigned stones, Position& position);
    unsigned getMaxStones();
    int endgameMinimax(Position& position, int alpha, int beta, unsigned maximizingPlayer);

    int setEvaluation(Position& position, bool player, int evaluation);
    int getEvaluation(Position& position, bool player);
private:

    /* 
     * If positive, evaluation is the number of moves till player 1 wins
     * If negative, evaluation is the number of moves till player 2 wins
     * If 0, position is a draw
     * If UNKNOWN, position hasn't been evaluated (should never happen for stones <= maxStones)
     */
    std::vector<int8_t> evaluations;

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
    std::atomic<unsigned> lookups{0};
    unsigned long long numJobs;

    unsigned maxStones;

    void* tree;
};