#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "table.c"

typedef struct TreeNode {
    uint16_t child_exists;
    uint16_t prefix_exists;
    char* next_hop_arr;
    struct TreeNode* child_block;
} TreeNode;

void setupNode(TreeNode* setMeUp){
    setMeUp->child_exists = 0;
    setMeUp->prefix_exists = 0;
    setMeUp->next_hop_arr = (char*) malloc(sizeof(char) * 15);
    setMeUp->child_block = (TreeNode*) malloc(sizeof(TreeNode) * 16);
}

TreeNode* constructTreeBitmap(struct TABLEENTRY* table, int tablelength){
    TreeNode* root = (TreeNode*) malloc(sizeof(TreeNode));
    setupNode(root);
    for(int i = 0; i < tablelength; i++){
        TreeNode* currentNode = root;
        uint16_t remaining_prefix = table[i].ip & 0xFFFF;
        int remaining_length = table[i].len - 16;
        int current_first_four_bits = (remaining_prefix & 0xF000) >> 12;
        

    }
    return root;
}



int main(){
    int tablelength;
    struct TABLEENTRY* table = set_table("ipv4/ipv4_rrc_all_90build.txt", &tablelength);
    constructTreeBitmap(table, tablelength);
    return 0;
}
