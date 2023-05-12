#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "table.c"
#include "util.c"

int countSetBits(uint16_t num) {
    int count = 0;
    while (num != 0) {
        num = num & (num - 1);
        count++;
    }
    return count;
}


int numberOfNodes = 0;

typedef struct TreeNode {
    uint16_t child_exists;
    uint16_t prefix_exists;
    char* next_hop_arr;
    struct TreeNode* child_block;
} TreeNode;

void setupNode(TreeNode* setMeUp){
    numberOfNodes++;
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
        int remaining_length = (int)table[i].len - 16;
        if(remaining_length < 1) continue; //skip the troublemakers
        int current_first_four_bits = (remaining_prefix & 0xF000) >> 12;
        while(remaining_length > 3){
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
        if(pos >= 15)printf("Warning: saving at %d\n", pos);
        currentNode->next_hop_arr[pos] = table[i].nexthop; //make actual entry
    }
    return root;
}

void compressNode(TreeNode* node){
    //compress next hop info
    int count = countSetBits(node->prefix_exists);
    char* new_next_hop_arr = (char*) malloc(sizeof(char) * count);
    int q = 0;
    for (int i = 0; i < 16; i++) {
        if ((node->prefix_exists >> i) & 1) {
            new_next_hop_arr[q++] = node->next_hop_arr[i];
        }
    }
    free(node->next_hop_arr);
    node->next_hop_arr = new_next_hop_arr;

    //compress child block
    count = countSetBits(node->child_exists);
    TreeNode* new_child_block = (TreeNode*) malloc(sizeof(TreeNode) * count);
    q = 0;
    for (int i = 0; i < 16; i++) {
        if ((node->child_exists >> i) & 1) {
            new_child_block[q++] = node->child_block[i];
        }
    }
    free(node->child_block);
    node->child_block = new_child_block;

    //recurse over compressed childblock
    for(int i = 0; i < count; i++){
        compressNode(node->child_block + i);
    }
}

int main(){
    int tablelength;
    uint64_t start, end;
    start = rdtsc();
    struct TABLEENTRY* table = set_table("ipv4/ipv4_rrc_all_90build.txt", &tablelength);
    end = rdtsc();
    printf("Elapsed clock cycles for building the table: %d\n", end-start);
    start = rdtsc();
    TreeNode* root = constructTreeBitmap(table, tablelength);
    end = rdtsc();
    printf("Elapsed clock cycles for building the TreeBitmap: %d with %d nodes.\n", end-start, numberOfNodes);
    printf("Clock cycles per node added: %f\n", (end-start)/(double)numberOfNodes);
    start = rdtsc();
    compressNode(root);
    end = rdtsc();
    printf("Elapsed clock cycles for table compression: %d\n", end-start);
    printf("Clock cycles per node: %f\n", (end-start)/(double)numberOfNodes);
    return 0;
}
