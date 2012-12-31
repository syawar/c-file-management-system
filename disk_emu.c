#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>



//cache class
struct Cache{
int address;
int dirtyData;
char *data;
struct Cache *next;
};
typedef struct Cache cache;
/*Variable Section-------------------*/
char *fileName;
int blockSize;
int maxBlocks;
int l=2;
int P=4;
FILE *myFile;
int prev;
int LRU;
int cacheSize = 0;
int mCache;
cache *head;
cache *previous;
cache *result;
cache *myLRU;
//Variable End---------------------------/


//helper Functions----------------/

int find(int s){
	cache *temp;
	temp = head;

	while(temp->next != NULL){
		if(temp->address == s){
		  result = temp;
		    return 0;
                }
                temp = (temp)->next;
	}

    return 1;
}

int fillCheck(){
	int num;
	cache *temp;
	temp = head;

	while(temp->next != NULL){
		if(temp->address == -1){
			num = gotoHead(temp);
			return 0;
		}
		temp = temp->next;
	}
return 1;

}

int findPrev(cache *current){

	cache *temp;
	temp = head;
	while(temp->next != NULL){
		if(temp->next == current){
			previous = temp;
			return 0;
		}
		temp = temp->next;
	}

	return 1;

}

int gotoHead(cache *current){

	if(current == head){

		return 0;
	}
	else{
		prev = findPrev(current);

		if(prev == 1){
			return 1;
		}

		previous->next = current->next;
		current->next = head;
		head = current;
		return 0;
	}

}

int getLRU(){
	cache *temp;
	temp = head;
	int i;

	for(i=1; i<cacheSize; i++){
		temp = temp->next;
	}
	myLRU = temp;
	return 0;
}



//End----------------------------/



int init_disk(char *filename, int block_size, int max_blocks, double L, double p){
    fileName=filename;
    blockSize=block_size;
    maxBlocks=max_blocks;
    int i;
    //to make sure that there is always some latency
    if(L>0){l=L;}
    if(p>0){P=p;}

    //open file and create null terminated string in it
    myFile = fopen(fileName, "w");
    for(i = 0; i< blockSize * maxBlocks; i++){
	fputc('0',myFile);
    }
    fclose(myFile);


    //check - Remove later
    printf("Initialised file=%s\n\n,fileName");


    return 0;
} 

int read_blocks(int start_address, int nblocks, void *buffer){
        void *buff1;
	void *buff2;
        cache *temp;
        int search_result;
	int cachefull;
	int blocks;
	int position;
        int i;

	buff1 = (char *) malloc(blockSize);
	buff2 = (char *) malloc(blockSize);
	myFile = fopen("new.dsk", "r+");
	if((start_address >= 0) &&(nblocks >= 0)){// valid
		if(cacheSize == 0){
			fseek (myFile, start_address*blockSize, SEEK_SET);
			blocks = fread(buffer, blockSize, nblocks, myFile);
                	fclose(myFile);
			return blocks;
		}
		for(i = 0; i < nblocks; i++){ // Non Empty Cache
			search_result = find(start_address);
			if(search_result == 1){//element not in cache
				cachefull = fillCheck();
				if (cachefull == 1){//cache full. cp disk to LRU

                                        printf("cache is full; copy from disk to LRU\n");
					LRU = getLRU();

					temp = head;
					position = gotoHead(myLRU);

					if(head->dirtyData == 1){ //dirty data copy to disk
						memcpy(buff2,head->data, blockSize);
						fseek (myFile, head->address * blockSize, SEEK_SET);
						fwrite(buff2, blockSize, 1, myFile);
						head->dirtyData=0;

					}
					fseek (myFile, start_address*blockSize, SEEK_SET);
					fread(buff1,blockSize, 1, myFile);
					memcpy(head->data,buff1,blockSize);
					head->address = start_address;
					memcpy(buffer,head->data,blockSize);
					buffer = buffer + blockSize;
					start_address = start_address + 1;
				}

                                else{ //data on new block
					printf("copying data on a new block\n");

					fseek (myFile, start_address*blockSize, SEEK_SET);
					fread(buff1,blockSize, 1, myFile);
					memcpy(head->data,buff1,blockSize);
					head->address = start_address;
					memcpy(buffer,head->data,blockSize);
					buffer = buffer + blockSize;
					start_address = start_address + 1;

				}
				fclose(myFile);

			}

			else{// element is in the cache
				printf("data present within the cache;\n");
				memcpy(buffer,result->data,blockSize);
				position = gotoHead(result);
				buffer = buffer + blockSize;
				start_address = start_address + 1;
				fclose(myFile);
			}
		}
		return nblocks;
	}
        else{// parameters invalid
            return -1;
	}
    
}

int write_blocks(int start_address, int nblocks, void *buffer){
        int search;
	int cacheCheck;
	int nloc;
        int i;
	void *tempbuffer;

	tempbuffer  = (char *) malloc(blockSize);


	myFile = fopen("server1.dsk", "r+");
	//myFile = fopen(myFilename, "r+");

	if((start_address >= 0) &&(nblocks >= 0)){// valid parameters


		if(cacheSize == 0){// cache is empty; write directly on disk
			//sleep(L);
			//srand((double)time(NULL));
			//if (rand() <= p) return 0;
			fseek (myFile, start_address*blockSize, SEEK_SET);
			fwrite(buffer, blockSize, nblocks, myFile);
			fclose(myFile);
			return nblocks;

		}

		for(i = 0; i<nblocks; i++){// cache is non empty; write on cache

			search = find(start_address);
			if(search == 1){//element is not in cache; write on a new block
				cacheCheck = fillCheck();

				if (cacheCheck == 1){//cache full write to LRU


                                        LRU = getLRU();
					nloc = gotoHead(myLRU);

					if(head->dirtyData == 1){ //dirty data cp to disk
						memcpy(tempbuffer,head->data, blockSize);
						fseek (myFile, head->address * blockSize, SEEK_SET);
						fwrite(tempbuffer, blockSize, 1, myFile);
						head->dirtyData=0;
					}

					head->address = start_address;
					head->data = buffer;
					head->dirtyData = 1;
					buffer = buffer + blockSize;
					start_address = start_address + 1;

				}

                                else{
					// cache is not full; write on a new block immediately
					head->address = start_address;
					head->data = buffer;
					head->dirtyData = 1;
					buffer = buffer + blockSize;
					start_address = start_address + 1;
				}
			}

                        else{// data in cache
				result->data = buffer;
				result->dirtyData = 1;
				nloc = gotoHead(result);
				buffer = buffer + blockSize;
				start_address = start_address + 1;
			}

		}
	}

	else{// invalid parameters
            return -1;
	}
}

int init_cache(int cache_size){
    cacheSize = cache_size;
    cache * tempStore;
    int i;
    head = (cache *) malloc(cache_size * sizeof(cache));
    tempStore = head;
    for(i=0;i<cache_size-1;i++){
	tempStore->data = (char *) malloc(blockSize);
        tempStore->address = -1;
        tempStore->dirtyData = 0;
        tempStore->next = tempStore+1;
	tempStore = tempStore->next;
    }

    //last one
    tempStore->data = (char *) malloc(blockSize);
    tempStore->address = -1;
    tempStore->dirtyData = 0;
    tempStore->next = NULL;
    return 0;
}

int flush_cache(void){
cache *temp;
temp = head;
int i;
void *tempbuf;
tempbuf  = (char *) malloc(blockSize);

for (i=0;i<blockSize;i++){
	if(head->dirtyData == 1){ //copy dirty on disk
			memcpy(tempbuf,temp->data, blockSize);
			fseek (myFile, temp->address * blockSize, SEEK_SET);
			fwrite(tempbuf, blockSize, 1, myFile);
			temp->dirtyData=0;
	}

}
return 0;
}
