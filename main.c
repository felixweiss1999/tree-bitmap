#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "table.c"

typedef struct TreeNode {
    uint16_t child_exists;
    uint16_t prefix_exists;
    uint32_t* hoparr;
    struct TreeNode* childblock;
} TreeNode;

int main(){
    struct TABLEENTRY* table = set_table("ipv4/ipv4_rrc_all_90build.txt");
    
    return 0;
}
