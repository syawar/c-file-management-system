
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#define BLOCK_SIZE 1024
#define NUM_BLOCKS 128
#define MAX_myfileS 20

//Variable section--------------------//
int blockSize;
int numBlocks;
int maximummyfiles;
int myfileCount;
int currmyfile;
int rootFAT;
int readLen;
int sizelist;
int FATindex;
int error = 0;
int writeLength;
char *list;
char *tempbuff;
char *tempbuff2;
char *tempbuff3;
char *namelist;
char *date;
char* readBuff;
char* appendBuff;
char* writeBuff;

//variable section end-----------------//

//entry stuct
struct dirEntry{
	char name[12];
	int size;
	int date;
	int indFAT;
};

//fdt entry struct
struct fdtEntry{
	int read;
	int write;
	int dirIndex;
};


typedef struct dirEntry entry;
struct fdtEntry FDT[MAX_myfileS];
entry directory[MAX_myfileS];
int FAT[NUM_BLOCKS][2];
char free_list[NUM_BLOCKS];
entry myfile;

//Helper Functions-------------//
int searchBlockSpace(){
	int i;

	for(i=0; i < 1024; i++){
            if(free_list[i] == '0'){
		return i;
            }
	}
	return -1;
}

int searchFAT(){
	int i,k;
	for(i=0; i<1024; i++){
            if(FAT[i][0]==-2){
                return i;
            }
	}
	return -1;
}

int findDirectory(){
	int i;
	for(i=0; i<MAX_myfileS; i++){
            if(directory[i].indFAT==-1){
                return i;
            }
	}
	return -1;
}

int findFDT(){
	int i;
	for(i=0; i<MAX_myfileS; i++){
		if(FDT[i].dirIndex ==-1){
                    return i;
                }
	}
	return -1;

}

int updateDisk(){
	buffer = (char *) malloc(1024);
	memcpy(buffer, directory, 1024);
	write_blocks(0, 1, buffer);
	memcpy(buffer, FAT, 1024);
	write_blocks(1, 1, buffer);
	memcpy(buffer, free_list, 1024);
	write_blocks(1023, 1, buffer);
	return 0;
}

//--helper function end---///

void mksfs(int fresh){
	int freeSpace;
	int FATSize;
	int dirSize;
        int i;
        int j;
	char *buffer1;
	blockSize = BLOCK_SIZE;
	numBlocks = NUM_BLOCKS;
	maximummyfiles = MAX_myfileS;
	buffer1 = (char *) malloc(1024);

	if(fresh != 0){//create a new sfs

		init_disk("s2.dsk", blockSize, numBlocks);
		init_cache(0);
		for(i=0; i<maximummyfiles; i++){
			strcpy(directory[i].name, "empty_myfile");
			directory[i].size = 0;
			directory[i].date = 0;
			directory[i].indFAT = -1;
		}

		for(i=0; i<numBlocks; i++){
			for(j=0; j<2; j++) FAT[i][j] = -2;
			}

		for(i=0; i<numBlocks; i++){
			free_list[i] = '0';
		}
		free_list[0] = '1';
		free_list[1] = '1';
		free_list[numBlocks - 1] = '1';

		//set FDT settings
		for(i=0; i<maximummyfiles; i++){
			FDT[i].dirIndex = -1;
		}

		dirSize = sizeof(entry) * maximummyfiles;
		FATSize = sizeof(int) * numBlocks * 2;
		freeSpace = sizeof(char) * numBlocks;

		while(dirSize>0){
			if(dirSize>1024){
				memcpy(buffer1, directory, 1024);
			}
			else {
				memcpy(buffer1, directory, dirSize);
			}
			write_blocks(0, 1, buffer1);
			dirSize -= 1024;
		}

		while(FATSize>0){
			if(FATSize>1024){
				memcpy(buffer1, FAT, 1024);
			}
			else {
				memcpy(buffer1, FAT, FATSize);
			}
			write_blocks(1, 1, buffer1);
			FATSize -= 1024;
		}

		while(freeSpace>0){
			if(freeSpace>1024){
				memcpy(buffer1, free_list, 1024);
			}
			else {
				memcpy(buffer1, free_list, freeSpace);
			}
			write_blocks(1023, 1, buffer1);
			freeSpace -= 1024;
		}

		printf("SFS created successfully\n");
	}

	else{ // load existing sfs

		for(i=0; i<maximummyfiles; i++){
			FDT[i].dirIndex = -1;
		}

		read_blocks(0, 1, buffer1);
		memcpy(directory,buffer1, 1024);
		read_blocks(1, 1, buffer1);
		memcpy(FAT,buffer1, 1024);
		read_blocks(1023, 1, buffer1);
		memcpy(free_list,buffer1, 1024);
		printf("SFS loading successfull\n");


	}

	
	read_blocks(0, 1, buffer1);

	memcpy(directory,buffer1, 1024);

	read_blocks(1, 1, buffer1);
	memcpy(FAT,buffer1, 1024);

	read_blocks(1023, 1, buffer1);
	memcpy(free_list,buffer1, 1024);
}

int sfs_open(char *name){
	int i;
	int blocksFree;
	int FATfree;
        int freeFdt;
	int DirFree;
        int DIRIndex;
	int exists = 0;

	for(i=0; i< MAX_myfileS;i++){
		if(strcmp(directory[i].name, name)==0){
			myfile = directory[i];
			DIRIndex = i;
			exists = 1;
		}
	}

	if(exists != 1){ //create and open new myfile

		blocksFree = searchBlockSpace();
		FATfree = searchFAT();
		DirFree = findDirectory();
		freeFdt = findFDT();
		strcpy(directory[DirFree].name, name);
		directory[DirFree].size = 0;
		directory[DirFree].date = time(NULL);
		directory[DirFree].indFAT = FATfree;
		FAT[FATfree][0] = blocksFree;
		FAT[FATfree][1] = EOF;
		FDT[freeFdt].read= 0;
		FDT[freeFdt].write= 0;
		FDT[freeFdt].dirIndex= DirFree;
		free_list[blocksFree] = '1';
		updateDisk();
		printf("%s <--Creation successfull\n", name);

	}

	else{ // open existing myfile
            freeFdt = findFDT();
            FDT[freeFdt].read= 0;
            FDT[freeFdt].write= myfile.size;
            FDT[freeFdt].dirIndex= DIRIndex;
            printf("%s <--Successfully Opened\n", name);

	}

	return freeFdt;
}

void sfs_write(int fd, char *buf, int length){
	int i;
        int position = 0;
        int maxLength;
	int apendLength;
        int FATi;
	int currFATi;
	int newBlock;

	writeLength = length;
	currmyfile = fd;
	writeBuff = (char *) malloc(length);
	appendBuff = (char *) malloc(1024 + length);
        for(i=0; i < writeLength; i++){
		writeBuff[i] = buf[i];
	}
	writeBuff[writeLength] = 0;
        myfile = directory[FDT[currmyfile].dirIndex];
	rootFAT = myfile.indFAT;
	currFATi = rootFAT;
        for(i=0; i < 1024; i++){//get last block in myfile
            if(FDT[currmyfile].write - position > 1024){
		position = position + 1024;
		currFATi = FAT[currFATi][1];
            }

	}
	read_blocks(FAT[currFATi][0], 1, appendBuff);
	appendBuff[FDT[currmyfile].write - position] = 0;
	maxLength = FDT[currmyfile].write + writeLength;
	strcat(appendBuff, writeBuff);
	apendLength = FDT[currmyfile].write - position + writeLength;
	appendBuff[apendLength] = 0; 

	for(i=0; i < apendLength;i++){
            if(apendLength - 1024 <= 0){
		write_blocks(FAT[currFATi][0],1,appendBuff);
		FAT[currFATi][1] = EOF; 
            }
            else{
                write_blocks(FAT[currFATi][0],1,appendBuff);
		appendBuff = appendBuff + 1024;
		apendLength = apendLength - 1024;
		newBlock = searchBlockSpace();
		FATi = searchFAT();
		if(newBlock > 0){
                    FAT[currFATi][1] = FATi;
                    FAT[FATi][0] = newBlock;
                    FAT[FATi][1] = EOF;
                    free_list[newBlock] = '1';
                    currFATi = FATi;
                }
		else{
                    error = 1;
		}
            }
	}
	if(error != 1){
            FDT[currmyfile].write = maxLength;
            directory[FDT[fd].dirIndex].date = time(NULL);
            directory[FDT[fd].dirIndex].size = directory[FDT[fd].dirIndex].size + length;
            updateDisk();
            free(writeBuff);
            free(appendBuff);
            printf("Data writing successfull\n");
	}
	else{
            printf("ERROR!!!--end of free blocks\n");
        }

}

void sfs_read(int fd, char *buf, int length){
	
	currmyfile = fd;
        int blockID = 1;
	int position = 0;
	int diff;
	int spaceLeft;
        int i,j;
	int currFATi;
        int temptr;

	readBuff = (char *) malloc(1024);
	tempbuff = (char *) malloc(1024);
	tempbuff2 = (char *) malloc(1024);
	tempbuff3 = (char *) malloc(1024);
	readLen = length;
	spaceLeft = readLen;
	myfile = directory[FDT[fd].dirIndex];
	rootFAT = myfile.indFAT;
	currFATi = rootFAT;
	for(i=0; i < 1024; i++){
            if(FDT[currmyfile].read - position > 1024){
		position = position + 1024;
		blockID = blockID + 1;
		currFATi = FAT[currFATi][1];
            }
	}

	diff = (blockID * 1024) - FDT[currmyfile].read;
	temptr = FDT[currmyfile].read;
        for(i=0; i < 1024;i++){
		if(readLen > diff){
                    read_blocks(FAT[currFATi][0],1,readBuff);
			for(j=0; j < diff; j++){
				tempbuff[j] = readBuff[j+FDT[currmyfile].read];
			}
			readLen = readLen - diff;
			temptr = temptr + diff;
			currFATi = FAT[currFATi][1];
			blockID = blockID + 1;
			position = position + 1024;
			diff = 1024;
                }
		else{
                    read_blocks(FAT[currFATi][0],1,readBuff);
                    if(FDT[currmyfile].read==0){
                        for(j=0; j < readLen; j++){
                            tempbuff[j] = readBuff[j+FDT[currmyfile].read];
			}
                    }
                    else{
			if(diff != 1024){
                            for(j=0; j < diff; j++){
                                tempbuff[j] = readBuff[j+FDT[currmyfile].read -(blockID-1)*1024];
                            }
			}
                        if(diff == 1024){
                            for(j=0; j < readLen; j++){
                                 tempbuff2[j] = readBuff[j];
				}
			}
                    }
		}
	}
	strcpy(buf,tempbuff);
	strcat(buf,tempbuff2);
	FDT[currmyfile].read = FDT[currmyfile].read + length;

}

void sfs_ls(){

	int i;
        char* sizeEl;
	char* FATel;
	char* table;

	namelist = (char *) malloc(20);
	date = (char *) malloc(30);
	table = (char *) malloc(10000);
	sizeEl = (char *) malloc(100);
	FATel = (char *) malloc(100);
	for(i=0; i < MAX_myfileS; i++){
            strcpy(namelist,directory[i].name);
	    ctime_r(&(directory[i].date),date);
            sizelist = directory[i].size;
            FATindex = directory[i].indFAT;
            if(directory[i].indFAT >= 0){
                strcat(namelist,"\t");
                sprintf(sizeEl,"%d\t", sizelist);
		sprintf(FATel,"%d\t", FATindex);
                strcat(table, namelist);
		strcat(table, sizeEl);
		strcat(table, FATel);
		strcat(table, date);
            }
	}
	
}

int sfs_close(int fd){
          FDT[fd].read= 0;
          FDT[fd].write= 0;
          FDT[fd].dirIndex = -1;
          printf("Close Successfull\n");
          return 0;
}

int sfs_remove(char *file){
	int exists = 0;
	int i, block;
	int DIRIndex;
        int temp = myfile.indFAT;
	for(i=0; i< MAX_myfileS;i++){
            if(strcmp(directory[i].name, file)==0){
		myfile = directory[i];
		DIRIndex = i;
		exists = 1;
            }
	}

	if(exists == 0){
		printf("ERROR: the file doesnot exist!!\n");
		return -1;
	}
        else{

            while(temp != EOF){
		block = FAT[temp][0];
		free_list[block] = '0';
		FAT[temp][0] = -2;
		temp = FAT[temp][1];
            }
            strcpy(directory[DIRIndex].name, "empty_file");
            directory[DIRIndex].size = 0;
            directory[DIRIndex].date = 0;
            directory[DIRIndex].indFAT = -1;

            for(i=0; i< MAX_myfileS;i++){//finds the myfile to be removed
		if (FDT[i].dirIndex == DIRIndex){
			FDT[i].dirIndex = -1;
			FDT[i].read= 0;
			FDT[i].write= 0;

		}
            }
            updateDisk();
            printf("%s<--Remove successfull\n");
            return 0;
            }

}


