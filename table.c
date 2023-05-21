#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

struct TABLEENTRY{
	unsigned int ip;
	unsigned char len;
	unsigned char nexthop;
};

struct AddressEntry{
    struct in6_addr address;
    int subnet_mask;
};

void read_table(char *str,unsigned int *ip,int *len,unsigned int *nexthop){
	char tok[]="./";
	char buf[100],*str1;
	unsigned int n[4];
	sprintf(buf,"%s\0",strtok(str,tok));
	n[0]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[1]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[2]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[3]=atoi(buf);
	*nexthop=n[2];
	str1=(char *)strtok(NULL,tok);
	if(str1!=NULL){
		sprintf(buf,"%s\0",str1);
		*len=atoi(buf);
	}
	else{
		if(n[1]==0&&n[2]==0&&n[3]==0)
			*len=8;
		else
			if(n[2]==0&&n[3]==0)
				*len=16;
			else
				if(n[3]==0)
					*len=24;
	}
	*ip=n[0];
	*ip<<=8;
	*ip+=n[1];
	*ip<<=8;
	*ip+=n[2];
	*ip<<=8;
	*ip+=n[3];
}

struct TABLEENTRY* set_table(char *file_name, int* num_entry){
	FILE *fp;
    *num_entry = 0;
	int len;
	char string[100];
	uint32_t ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		(*num_entry)++;
	}
	rewind(fp);
	struct TABLEENTRY* table = (struct TABLEENTRY *)malloc((*num_entry)*sizeof(struct TABLEENTRY));
	*num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		table[*num_entry].ip=ip;
		table[*num_entry].nexthop=nexthop;
		table[(*num_entry)++].len=len;
	}
    return table;
}

unsigned int* set_query_table(char *file_name, int* num_entry){
	FILE *fp;
	int len;
	char string[100];
	*num_entry = 0;
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		(*num_entry)++;
	}
	rewind(fp);
	unsigned int* query = (unsigned int*) malloc((*num_entry)*sizeof(unsigned int));
	*num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
        query[(*num_entry)++] = ip;
	}
	return query;
}

int countLines(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file '%s'\n", filename);
        return 0;
    }

    int num_lines = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            num_lines++;
        }
    }

    fclose(file);
    return num_lines;
}

unsigned char* getIPv6AddressChunks(const struct AddressEntry* addresses, int index, unsigned char* address_chunks) {
    if (address_chunks == NULL) {
        printf("Failed to allocate memory for address chunks.\n");
        return NULL;
    }

    const unsigned char* address_bytes = (const unsigned char*) &(addresses[index]);
    for (int i = 0; i < 16; i++) {
        address_chunks[i * 2] = address_bytes[i] >> 4;      // Get the high 4 bits
        address_chunks[i * 2 + 1] = address_bytes[i] & 0x0F; // Get the low 4 bits
    }

    return address_chunks;
}

struct AddressEntry* read_ipv6_table(char* filename, int* num_addresses){
    *num_addresses = countLines(filename);
    struct AddressEntry* addresses = (struct AddressEntry*) malloc(sizeof(struct AddressEntry) * (*num_addresses));
    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file '%s'\n", filename);
        return addresses;
    }
	
    // Read addresses from the file
    char line[500];
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL && index < (*num_addresses)) {
        line[strcspn(line, "\n")] = '\0';  // Remove the newline character

        // Parse the address and subnet mask
        char* address_str = strtok(line, "/");
        char* mask_str = strtok(NULL, "/");
        if (address_str != NULL && mask_str != NULL) {
            // Convert address string to binary form
            struct AddressEntry entry;
            if (inet_pton(AF_INET6, address_str, &(entry.address)) == 1) {
                // Convert subnet mask string to integer
                entry.subnet_mask = atoi(mask_str);

                addresses[index] = entry;
                index++;
            }
        }
    }

    // Close the file
    fclose(file);

	return addresses;
}