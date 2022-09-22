#pragma once
#include "position.hpp"
#include <cstdint>

struct TreeNode
{
    union
    {
        // N = number of stones
        // array of N children
        TreeNode* children;
        // array of N evaluations TODO make this an array of size 2N for player 1 and player 2
        int8_t* evaluations;
    };
};

class EndgameTable
{
public:
    virtual void setEvaluation(Position& position, int player1Evaluation, int player2Evaluation) = 0;
    virtual int getEvaluation(Position& position, bool player) = 0;
    virtual void init() = 0;

    // maximum number of stones for which every endgame is evaluated
    unsigned mMaxStones = 0;
};


