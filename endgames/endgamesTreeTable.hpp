#pragma once
#include "endgamesTable.hpp"
#include "position.hpp"
#include <atomic>
#include <cstddef>


class EndgameTreeTable : public EndgameTable
{
public:
    EndgameTreeTable(unsigned totalStones);
    void init() override;
    void setEvaluation(Position& position, int player1Evaluation, int player2Evaluation) override;
    int getEvaluation(Position& position, bool player) override;

private:
    void buildTree();
    void buildTreePlayer2(unsigned player2MaxStones, TreeNode* node, Position& position);
    TreeNode* newTreeNodes(unsigned numNodes);

    TreeNode root;

    /* Pre-allocated memory to avoid constantly needing to assign new memory */
    // ints for evaluations
    int8_t* mIntArray;
    std::atomic<unsigned> mIntArrayIndex{0};

    TreeNode* mTreeNodeArray;
    std::atomic<unsigned> mTreeNodeArrayIndex{0};

    // maximum number of stones for which memory is assigned
    unsigned mTotalStones;
    // maximum number of stones for which evaluations have been computed
    unsigned mMaxStones;
};


