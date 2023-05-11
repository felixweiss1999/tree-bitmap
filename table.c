#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct TABLEENTRY{
	unsigned int ip;
	unsigned char len;
	unsigned char nexthop;
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