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
int numberOfNextHopsStored = 0;

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

unsigned char extract_shift_decrement4(uint32_t* prefix, uint32_t* remainingLength) {
    unsigned int mask = 0xFFFFFFFF;  // Mask to extract all bits
    unsigned char ret;
    if ((*remainingLength) < 4) {
        mask = (0xF0000000 << (4 - (*remainingLength)));  // Update the mask based on remaining length
        *prefix = (*prefix) << (*remainingLength);
        ret = ((*prefix) & mask) >> (32 - (*remainingLength));
        (*remainingLength) = 0;
    } else {
        mask = (0xF0000000);
        *prefix = (*prefix) << 4;
        ret = ((*prefix) & mask) >> 28;
        (*remainingLength) -= 4;
    }
    return ret;
}


TreeNode* constructTreeBitmap(TreeNode* root, struct TABLEENTRY* table, int tablelength){
    for(int i = 0; i < tablelength; i++){
        TreeNode* currentNode = root;
        uint32_t remaining_prefix = table[i].ip;
        int remaining_length = (int)table[i].len;
        unsigned char current_first_four_bits = extract_shift_decrement4(&remaining_prefix, &remaining_length);
        while(remaining_length > 3){
            if(!((currentNode->child_exists >> current_first_four_bits) & 1)){ //child doesn't exist yet?
                currentNode->child_exists |= (1 << current_first_four_bits); //set bit at corresponding position to indicate child now exists
                setupNode(currentNode->child_block + current_first_four_bits);
            }
            currentNode = currentNode->child_block + current_first_four_bits;
            current_first_four_bits = extract_shift_decrement4(&remaining_prefix, &remaining_length);
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
    numberOfNextHopsStored += count;
    if(count < 15){
        char* new_next_hop_arr = (char*) malloc(sizeof(char) * count);
        int q = 0;
        for (int i = 0; i < 16; i++) {
            if ((node->prefix_exists >> i) & 1) {
                new_next_hop_arr[q++] = node->next_hop_arr[i];
            }
        }
        free(node->next_hop_arr);
        node->next_hop_arr = new_next_hop_arr;
    }
    
    //compress child block
    count = countSetBits(node->child_exists);
    if(count < 16){
        TreeNode* new_child_block = (TreeNode*) malloc(sizeof(TreeNode) * count);
        int q = 0;
        for (int i = 0; i < 16; i++) {
            if ((node->child_exists >> i) & 1) {
                new_child_block[q++] = node->child_block[i];
            }
        }
        free(node->child_block);
        node->child_block = new_child_block;
    }
    

    //recurse over compressed childblock
    for(int i = 0; i < count; i++){
        compressNode(node->child_block + i);
    }
}

int searchPrefixBitmap(uint32_t prefix_exists, char byte){
    int pos;
    for(int r = 3; r >= 0; r--){
        pos = (1 << r) - 1 + byte;
        if((prefix_exists >> pos) & 1)
            return pos;
        byte = byte >> 1;
    }
    return -1;
}

int countSetBitsUpToP(uint16_t num, int p) { //not including position p!
    int count = 0;
    uint16_t mask = 1;
    
    for (int i = 0; i < p; i++) {
        if (num & mask) {
            count++;
        }
        mask <<= 1;
    }
    
    return count;
}

unsigned char* lookupIP(TreeNode* node, uint32_t ip){
    unsigned char* longestMatch = NULL;

    while(1){

        char first_four_bits = (ip >> 28);
        //check internal bitmap

        int pos = searchPrefixBitmap(node->prefix_exists, first_four_bits >> 1); //expects only last 3 bits to be relevant


        if(pos != -1){
            longestMatch = node->next_hop_arr + countSetBitsUpToP(node->prefix_exists, pos); //now points to best known next hop
        }
        //check external bitmap
        if(!((node->child_exists >> first_four_bits) & 1)) //there is no valid child potentially storing longer matching prefixes!
            return longestMatch;

        node = node->child_block + countSetBitsUpToP(node->child_exists, first_four_bits);


        ip = ip << 4;
    }
}

int main(){
    int tablelength;
    uint64_t start, end;
    TreeNode* root = (TreeNode*) malloc(sizeof(TreeNode));
    setupNode(root);
    //build
    start = rdtsc();
    struct TABLEENTRY* table = set_table("ipv4/ipv4_rrc_all_90build.txt", &tablelength);
    end = rdtsc();
    printf("Elapsed clock cycles for building the table: %d\n", end-start);
    



    start = rdtsc();
    root = constructTreeBitmap(root, table, tablelength);
    end = rdtsc();
    printf("Elapsed clock cycles for building the (uncompressed) TreeBitmap: %d with %d nodes.\n", end-start, numberOfNodes);
    printf("Clock cycles per node: %f\n", (end-start)/(double)numberOfNodes);
    printf("Clock cycles per prefix stored: %f\n", (end-start)/(double)tablelength);
    
    
    
    //insert
    table = set_table("ipv4/ipv4_rrc_all_10insert.txt", &tablelength);
    start = rdtsc();
    root = constructTreeBitmap(root, table, tablelength);
    end = rdtsc();
    printf("Elapsed clock cycles for inserting: %d with %d nodes.\n", end-start, numberOfNodes);
    printf("Clock cycles per insert: %f\n", (end-start)/(double)tablelength);
    
    
    
    //compress
    start = rdtsc();
    compressNode(root);
    end = rdtsc();
    printf("Elapsed clock cycles for table compression: %d\n", end-start);
    printf("Clock cycles per prefix stored: %f\n", (end-start)/(double)tablelength);
    printf("Total nexthops stored: %d\n", numberOfNextHopsStored);
    
    return 0;
    
    //query
    uint64_t totalclock = 0;
    int maxclock = 0;
    int minclock = 1000000000;
    uint32_t* query_table = set_query_table("ipv4/ipv4_rrc_all_10insert.txt", &tablelength);
    for(int i = 0; i < tablelength; i++){
        start = rdtsc();
        unsigned char* nexthop = lookupIP(root, query_table[i]);
        end = rdtsc();
        totalclock += end-start;
        if((end-start) > maxclock){
            //printf("maxclock %d for i=%d nexthop=%d t1=%llu t2=%llu iters=%llu\n", end-start, i, *nexthop, timestamp, timestamp2, iters);
            maxclock = end-start;
        }
        if((end-start) < minclock)
            minclock = end-start;
    }
    printf("Average clocks per lookup: %f\n", totalclock/(double)tablelength);
    printf("Maxclock: %d Minclock: %d\n", maxclock, minclock);
    printf("%d", sizeof(TreeNode));
    return 0;
}
