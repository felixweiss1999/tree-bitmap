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
        if(remaining_length < 1) continue; //skip the troublemakers
        int current_first_four_bits = (remaining_prefix & 0xF000) >> 12;
        while(remaining_length > 4){
            if(!((currentNode->child_exists >> current_first_four_bits) & 1)){ //child doesn't exist yet?
                currentNode->child_exists |= (1 << current_first_four_bits); //set bit at corresponding position to indicate child now exists
                setupNode(currentNode->child_block + current_first_four_bits);
            }
            currentNode = currentNode->child_block + current_first_four_bits;
            remaining_prefix = remaining_prefix << 4;
            remaining_length -= 4;
            current_first_four_bits = (remaining_prefix & 0xF000) >> 12;
        }
        //arrived at final node where nexthop info needs to be stored.
        int remainder_bits = (current_first_four_bits) >> (4 - remaining_length);
        int pos = (1 << remaining_length) - 1 + remainder_bits; //2^remaining_length - 1 equals the amount of classes of lower length
        currentNode->prefix_exists |= (1 << pos); //set bit at position to indicate prefix stored
        currentNode->next_hop_arr[pos] = table[i].nexthop; //make actual entry
    }
    return root;
}



int main(){
    int tablelength;
    struct TABLEENTRY* table = set_table("ipv4/ipv4_rrc_all_90build.txt", &tablelength);
    constructTreeBitmap(table, tablelength);
    return 0;
}
