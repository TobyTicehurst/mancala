#include "endgamesTreeTable.hpp"
#include <cstring>
#include <cstdio>


// TODO put these in utils
static inline unsigned long long numPositions(unsigned long long n)
{
    // Stars and bars with n Stars (Stones) and 11 bars (12 Slots) minus the positions where one side has 0 stones

    // have to use this method to not overflow an unsigned long long ((n + 11)! is massive when n = 48, but (n + 11)!/n! just fits until n = 50)
    return
    ((n + 11ull) * (n + 10ull) * (n + 9ull) * (n + 8ull) * (n + 7ull) * (n + 6ull) * (n + 5ull) * (n + 4ull) * (n + 3ull) * (n + 2ull) * (n + 1ull) /
    (unsigned long long)(11 * 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2)) -
    (unsigned long long)(2 * (n + 5) * (n + 4) * (n + 3) * (n + 2) * (n + 1) / (5 * 4 * 3 * 2));
}

static inline unsigned long long totalNumPositions(unsigned long long n)
{
    unsigned long long ret = 0;
    for (unsigned long long i = 2; i <= n; i++)
    {
        ret += numPositions(i);
    }
    return ret;
}

// (5+n)!/(n!*5!)
static inline unsigned long long temp(unsigned long long n)
{
    return (n + 5ull) * (n + 4ull) * (n + 3ull) * (n + 2ull) * (n + 1ull) / (5 * 4 * 3 * 2);
}

static inline unsigned long long treeGuess(unsigned long long n)
{
    unsigned long long numNodes = temp(n);
    for (unsigned long long i = 0; i < n - 1; i++)
    {
        numNodes += temp(i + 1) * temp(n - i);
    }
    return numNodes;
}

EndgameTreeTable::EndgameTreeTable(unsigned totalStones) :
    mTotalStones{totalStones},
    mMaxStones{0}
{}

void EndgameTreeTable::init()
{
    // assign memory (should this be done in init as new can throw?)
    size_t treeMemory = treeGuess(mTotalStones);
    size_t intMemory = 2 * totalNumPositions(mTotalStones);

    printf("Tree Node Memory: %fGB\n", (double)treeMemory / (double)1e9);
    printf("Evaluation Memory: %fGB\n", (double)intMemory / (double)1e9);

    // ensure we don't overflow the 32 bit indices
    if (treeMemory > (1ull << 32) || intMemory > (1ull << 32))
    {
        printf("Memory requirement too large!\n");
        return;
    }

    mTreeNodeArray = new TreeNode[treeMemory];
    mIntArray = new int8_t[intMemory];

    buildTree();
}

void EndgameTreeTable::setEvaluation(Position& position, int player1Evaluation, int player2Evaluation)
{
    unsigned index = root.
    children[position.slots[0]].
    children[position.slots[1]].
    children[position.slots[2]].
    children[position.slots[3]].
    children[position.slots[4]].
    children[position.slots[5]].
    children[position.slots[7]].
    children[position.slots[8]].
    children[position.slots[9]].
    children[position.slots[10]].
    children[position.slots[11]].
    evaluationsIndex;

    mIntArray[index + position.slots[12] * 2] = player1Evaluation;
    mIntArray[index + position.slots[12] * 2 + 1] = player2Evaluation;
}

int EndgameTreeTable::getEvaluation(Position& position, bool player)
{
    unsigned index = root.
    children[position.slots[0]].
    children[position.slots[1]].
    children[position.slots[2]].
    children[position.slots[3]].
    children[position.slots[4]].
    children[position.slots[5]].
    children[position.slots[7]].
    children[position.slots[8]].
    children[position.slots[9]].
    children[position.slots[10]].
    children[position.slots[11]].
    evaluationsIndex;

    return mIntArray[index + position.slots[12] * 2 + player];    
}

TreeNode* EndgameTreeTable::newTreeNodes(unsigned numNodes)
{
    TreeNode* nodeMemory = &mTreeNodeArray[mTreeNodeArrayIndex];
    mTreeNodeArrayIndex += numNodes;
    return nodeMemory;
}

void EndgameTreeTable::buildTree()
{
    printf("Building tree...\n");
    
    TreeNode* current[6];

    Position position;
    memset(&position, 0, sizeof(position));

    // number of stones still to be placed for each slot
    unsigned remainders[6];

    /* For each player 1 slot */

    // slot 0
    // max stones for slot 0
    remainders[0] = mTotalStones - 1;
    // root node
    root.children = newTreeNodes(remainders[0] + 1);

    // for each of these stones
    for (position.slots[0] = 0; position.slots[0] <= remainders[0]; position.slots[0]++)
    {
        current[0] = &root.children[position.slots[0]];
        // slot 1
        remainders[1] = remainders[0] - position.slots[0];
        // assign memory for children
        current[0]->children = newTreeNodes(remainders[1] + 1);

        for (position.slots[1] = 0; position.slots[1] <= remainders[1]; position.slots[1]++)
        {
            current[1] = &current[0]->children[position.slots[1]];
            // slot 2
            remainders[2] = remainders[1] - position.slots[1];
            // assign memory for children
            current[1]->children = newTreeNodes(remainders[2] + 1);

            for (position.slots[2] = 0; position.slots[2] <= remainders[2]; position.slots[2]++)
            {
                current[2] = &current[1]->children[position.slots[2]];
                // slot 3
                remainders[3] = remainders[2] - position.slots[2];
                // assign memory for children
                current[2]->children = newTreeNodes(remainders[3] + 1);

                for (position.slots[3] = 0; position.slots[3] <= remainders[3]; position.slots[3]++)
                {
                    current[3] = &current[2]->children[position.slots[3]];
                    // slot 1
                    remainders[4] = remainders[3] - position.slots[3];
                    // assign memory for children
                    current[3]->children = newTreeNodes(remainders[4] + 1);

                    for (position.slots[4] = 0; position.slots[4] <= remainders[4]; position.slots[4]++)
                    {
                        current[4] = &current[3]->children[position.slots[4]];
                        // slot 5
                        remainders[5] = remainders[4] - position.slots[4];
                        // assign memory for children
                        current[4]->children = newTreeNodes(remainders[5] + 1);

                        for (position.slots[5] = 0; position.slots[5] <= remainders[5]; position.slots[5]++)
                        {
                            current[5] = &current[4]->children[position.slots[5]];
                            // only true if all slots are 0
                            if (remainders[5] - position.slots[5] == mTotalStones - 1)
                                continue;
                            unsigned player2MaxStones = 1 + remainders[5] - position.slots[5];
                            buildTreePlayer2(player2MaxStones, current[5], position);
                        }
                    }
                }
            }
        }
    }
}

void EndgameTreeTable::buildTreePlayer2(unsigned player2MaxStones, TreeNode* node, Position& position)
{
    // number of stones still to be placed for each slot
    unsigned remainders[6];
    TreeNode* current[6];

    remainders[0] = player2MaxStones;
    node->children = newTreeNodes(remainders[0] + 1);

    for (position.slots[7] = 0; position.slots[7] <= remainders[0]; position.slots[7]++)
    {
        current[0] = &node->children[position.slots[7]];
        remainders[1] = remainders[0] - position.slots[7];
        // assign memory for children
        current[0]->children = newTreeNodes(remainders[1] + 1);

        for (position.slots[8] = 0; position.slots[8] <= remainders[1]; position.slots[8]++)
        {
            current[1] = &current[0]->children[position.slots[8]];
            remainders[2] = remainders[1] - position.slots[8];
            // assign memory for children
            current[1]->children = newTreeNodes(remainders[2] + 1);
                
            for (position.slots[9] = 0; position.slots[9] <= remainders[2]; position.slots[9]++)
            {
                current[2] = &current[1]->children[position.slots[9]];
                remainders[3] = remainders[2] - position.slots[9];
                // assign memory for children
                current[2]->children = newTreeNodes(remainders[3] + 1);
                
                for (position.slots[10] = 0; position.slots[10] <= remainders[3]; position.slots[10]++)
                {
                    current[3] = &current[2]->children[position.slots[10]];
                    remainders[4] = remainders[3] - position.slots[10];
                    // assign memory for children
                    current[3]->children = newTreeNodes(remainders[4] + 1);
                    
                    for (position.slots[11] = 0; position.slots[11] <= remainders[4]; position.slots[11]++)
                    {
                        current[4] = &current[3]->children[position.slots[11]];
                        remainders[5] = remainders[4] - position.slots[11];

                        // printf("%d\n", (unsigned)mTreeNodeArrayIndex);
                        // PrintPosition(position);

                        // number of evals needed to be stored at this point is:
                        // (remaining stones + 1) * 2
                        // unless all slots are 0
                        if (remainders[5] == player2MaxStones)
                        {
                            current[4]->evaluationsIndex = mIntArrayIndex - 1;
                            mIntArrayIndex += ((remainders[5]) * 2);
                        }
                        else
                        {
                            current[4]->evaluationsIndex = mIntArrayIndex;
                            mIntArrayIndex += ((remainders[5] + 1) * 2);
                        }
                    }
                }
            }
        }
    }
}

//TODO make more readable, but performance check. 1 function per for loop
