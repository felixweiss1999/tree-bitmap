#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WORD_SIZE 32 // assuming 32-bit machine

typedef struct TreeNode {
    uint32_t bitmap; // bitmap of the node
    int height; // height of the node in the tree
    struct TreeNode* left; // left child node
    struct TreeNode* right; // right child node
} TreeNode;

// create a new tree node with the given bitmap and height
TreeNode* createNode(uint32_t bitmap, int height) {
    TreeNode* node = (TreeNode*) malloc(sizeof(TreeNode));
    node->bitmap = bitmap;
    node->height = height;
    node->left = NULL;
    node->right = NULL;
    return node;
}

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

// insert a new element with the given value into the tree rooted at the given node
void insert(TreeNode** nodePtr, int value) {
    TreeNode* node = *nodePtr;
    if (node == NULL) {
        *nodePtr = createNode(0, 0);
        setBit(*nodePtr, value);
        return;
    }
    int heightDiff = 0;
    if (value < WORD_SIZE / 2) {
        insert(&node->left, value);
        heightDiff = node->left->height - node->height;
        if (heightDiff == 1) {
            if (node->left->bitmap == (1 << WORD_SIZE / 2) - 1) {
                node->left = createNode(0, node->height + 1);
                node->left->left = createNode(0, node->height);
                node->left->right = createNode(0, node->height);
                node->left->left->bitmap = (1 << WORD_SIZE / 2) - 1;
                node->left->right->bitmap = node->bitmap & ((1 << WORD_SIZE / 2) - 1);
                node->bitmap = node->bitmap >> WORD_SIZE / 2;
                node->left->right->bitmap = node->left->right->bitmap >> WORD_SIZE / 2;
            }
        }
    } else {
        insert(&node->right, value - WORD_SIZE / 2);
        heightDiff = node->right->height - node->height;
        if (heightDiff == 1) {
            if (node->right->bitmap == (1 << WORD_SIZE / 2) - 1) {
                node->right = createNode(0, node->height + 1);
                node->right->left = createNode(0, node->height);
                node->right->right = createNode(0, node->height);
                node->right->left->bitmap = node->bitmap >> WORD_SIZE / 2;
                node->right->right->bitmap = (1 << WORD_SIZE / 2) - 1;
                node->bitmap = node->bitmap & ((1 << WORD_SIZE / 2) - 1);
                node->right->left->bitmap = node->right->left->bitmap & ((1 << WORD_SIZE / 2) - 1);
            }
        }
    }
    if (heightDiff == 0) {
        setBit(node, value % WORD_SIZE);
    }
}