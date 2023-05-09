#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*
links to consider:
https://raminaji.wordpress.com/tree-bitmap/
*/
#define WORD_SIZE 32 // assuming 32-bit machine

typedef struct TreeNode {
    uint16_t child_exists; // child node existence bitmap
    uint16_t prefix_exists; // prefix bitmap
    uint32_t* hoparr; // pointer to nexthop array
    TreeNode* childblock; // pointer to child block
} TreeNode;

// set the ith bit of the bitmap of the given node
void setBit(TreeNode* node, int i) {
    node->bitmap |= (1 << i);
}

// clear the ith bit of the bitmap of the given node
void clearBit(TreeNode* node, int i) {
    node->bitmap &= ~(1 << i);
}

// check if the ith bit of the bitmap of the given node is set
int getBit(TreeNode* node, int i) {
    return (node->bitmap >> i) & 1;
}

// count the number of set bits in the bitmap of the given node
int countBits(TreeNode* node) {
    int count = 0;
    for (int i = 0; i < WORD_SIZE; i++) {
        count += getBit(node, i);
    }
    return count;
}

// get the rank of the ith bit in the bitmap of the given node
int rank(TreeNode* node, int i) {
    int rank = 0;
    for (int j = 0; j <= i; j++) {
        rank += getBit(node, j);
    }
    return rank;
}

// find the position of the rth set bit in the bitmap of the given node
int select(TreeNode* node, int r) {
    int count = 0;
    for (int i = 0; i < WORD_SIZE; i++) {
        if (getBit(node, i)) {
            count++;
            if (count == r) {
                return i;
            }
        }
    }
    return -1;
}

