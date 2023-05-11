#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct TreeNode {
    uint16_t child_exists;
    uint16_t prefix_exists;
    uint32_t* hoparr;
    TreeNode* childblock;
} TreeNode;


