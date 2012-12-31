#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>        
#include <unistd.h>          
#include <string.h>
#include <stdio.h>
#include "helper.h"

#define maxServ	 3
#define maxOpen	 1000
#define maxLine  1000

//FD entry struct----//
struct FDentry{
	int local;
	int remote;
	char server[12];
	char filename[12];
};
typedef struct FDentry FDentry;

//mount entry struct------//
struct mtentry{
        int myPort;
	int mySocket;
	char name[12];
	char hostIP[10];

};
typedef struct mtentry mtentry;


mtentry mount_table[maxServ];
FDentry OFD_table[maxOpen];

//variable section-------//
struct sockaddr_in servaddr;

char buffer[maxLine*10];
char *Sadress;
char *elcurrentserver;
char *el_table;
char *myServ;
char *Sport;
char *endptr;
char *header;
char *recEi;
char *recData;
char *sendData;

int port;
int recievedSize;
int size;
int recievedReq;
int conS;
int lsCom = 1;
int openCom = 2;
int closeCom = 3;
int writeCom= 4;
int readCom= 5;
int remCom= 6;
int request;

//variable section ends---//

//helper funtions----//
int ParseCmdLine(int argc, char *argv[], char **Sadress, char **Sport) {

    int n = 1;

    while ( n < argc ) {
	if ( !strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2) ) {
	    *Sadress = argv[++n];
	}
	else if ( !strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2) ) {
	    *Sport = argv[++n];
	}
	else if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) ) {
	    printf("Use:\n\n");
	    printf("timeclnt -a (remote IP) -p (remote port)\n\n");
	    exit(EXIT_SUCCESS);
	}
	++n;
    }

    return 0;
}

int sending(int elsock, char *echoString, int length){
	int numsent;
	int mylength = length;

	numsent = send(elsock, echoString, mylength, 0);
	while(numsent != mylength){
		mylength = mylength - numsent;
		numsent = send(elsock, echoString, mylength, 0);
	}
	return 0;
}

int recieveing(int elsock, char *echoString, int length){
	
	int numreceived;
	int mylength = length;

	numreceived = recv(elsock, echoString, mylength, 0);
	while(numreceived != mylength){
		mylength = mylength - numreceived;
		numreceived = recv(elsock, echoString, mylength, 0);
	}
	return 0;
}

int mountEntry(){
	int i;
	for(i=0; i<maxServ; i++){
		if(mount_table[i].myPort == -1) return i;
	}
	return -1;

}
void ofdInit(){
	int i;
	for(i=0; i<maxOpen; i++){
		OFD_table[i].local = -1;
	}
}

int ofdSearch(char *filesys, char *filename){
	int i;
	for(i=0; i<maxOpen; i++){
		if(strcmp(OFD_table[i].server,filesys) == 0){
			if(strcmp(OFD_table[i].filename,filename) == 0)
				return i;
		}
	}
	return -1;
}

void mountInit(){
	int i;
	for(i=0; i<maxServ; i++){
		mount_table[i].myPort = -1;
	}
}

int MTsearch(char *filesys){
	int i;
	for(i=0; i<maxServ; i++){
		if(strcmp(mount_table[i].name,filesys) == 0) return i;
	}
	return -1;

}

int fdSearch(){
	int i;
	for(i=0; i<maxOpen; i++){
		if(OFD_table[i].local == -1) return i;
	}
	return -1;

}



//---helper func end----//




void nfs_umount(char *filesys, char *host, int port){
    int MTloc;
    MTloc = mountEntry();

    if(MTloc != -1){
        mount_table[MTloc].myPort = port;
	strcpy(mount_table[MTloc].name, filesys);
        strcpy(mount_table[MTloc].hostIP, host);
        if ( (mount_table[MTloc].mySocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            fprintf(stderr, "Error!!\n");
            exit(EXIT_FAILURE);
	}
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(mount_table[MTloc].hostIP);
	servaddr.sin_port = htons(mount_table[MTloc].myPort);
        if ( connect(mount_table[MTloc].mySocket, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ) {
            printf("ECHOCLNT: Error calling connect()\n");
            exit(EXIT_FAILURE);
        }
	printf("%s Mounted:\n", mount_table[MTloc].name);

	
    }
    else{
	printf("unable to mount file system!!");
    }
}

void nfs_ls(char *filesys){
	
    if(MTsearch(filesys) != -1){
        size = 0;
        request = lsCom;
	header = (char *) malloc(2* sizeof(int));
	conS = mount_table[MTsearch(filesys)].mySocket;
        memcpy(header, &request, sizeof(int));
	memcpy(header+sizeof(int), &size, sizeof(int));
	sending(conS, header, 2*sizeof(int));
        recEi = (char *) malloc(2 * sizeof(int));
        recieveing(conS, recEi, 2 * sizeof(int));
        memcpy(&recievedReq, recEi, sizeof(int));
	memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));
        if(recievedReq == lsCom){
            if(recievedSize != 0){
		recData = (char *) malloc(recievedSize);
		el_table = (char *) malloc(recievedSize);
		recieveing(conS, recData, recievedSize);
		memcpy(el_table, recData, recievedSize);
		puts(el_table);
		free(recData);
            }
        }
    }
    else{
       printf("Error: %s not mounted\n", filesys);
    }
}

int nfs_fopen(char *filesys, char *name){
    char *myName;
    int ofdIn;
    int myRem;

    myName = (char *)malloc(strlen(name));
    strcpy(myName,name);
    myServ = (char *)malloc(strlen(filesys));
    strcpy(myServ,filesys);
    if(MTsearch(myServ) == -1){
		printf("Error: %s not mounted\n", filesys);
    }
    else{
        if( ofdSearch(myServ,myName) != -1){
			
	}
        else{
            ofdIn = fdSearch();
            if(ofdIn == -1){
		printf("unable to open more files\n");
            }
            else{
		request = openCom;
		size = strlen(myName);
		header = (char *) malloc(2* sizeof(int));
		conS = mount_table[MTsearch(filesys)].mySocket;
		memcpy(header, &request, sizeof(int)); 
		memcpy(header+sizeof(int), &size, sizeof(int));
		sending(conS, header, 2*sizeof(int));
		sendData = (char *)malloc(strlen(name));
		strcpy(sendData,myName);
		sending(conS, sendData, strlen(sendData));		
		recEi = (char *) malloc(2 * sizeof(int));
		recieveing(conS, recEi, 2 * sizeof(int));
		memcpy(&recievedReq, recEi, sizeof(int));
		memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));
		if(recievedReq == openCom){
			recData = (char *) malloc(recievedSize);
			recieveing(conS, recData, recievedSize);
			memcpy(&myRem, recData, sizeof(int));		
			OFD_table[ofdIn].local = ofdIn;
			OFD_table[ofdIn].remote = myRem;
			strcpy(OFD_table[ofdIn].server, filesys);
			strcpy(OFD_table[ofdIn].filename, name);
			free(myName);
			free(recEi);
			free(sendData);
                        free(header);
			free(myServ);
			return ofdIn;
		}

		else{
                    return -1;
					
		}

            }
        }
    }
}

void nfs_fclose(int fd){

    if(OFD_table[fd].local == fd){
       request = closeCom;
       size = 4;
       elcurrentserver = (char *) malloc (12);
       header = (char *) malloc(2* sizeof(int));
       strcpy(elcurrentserver,OFD_table[fd].server);
       conS = mount_table[MTsearch(elcurrentserver)].mySocket;
       memcpy(header, &request, sizeof(int));
       memcpy(header+sizeof(int), &size, sizeof(int));
        sending(conS, header, 2*sizeof(int));
        sendData = (char *)malloc(sizeof(int));
	memcpy(sendData, &(OFD_table[fd].remote), sizeof(int));
	sending(conS, sendData, sizeof(int));
        recEi = (char *) malloc(2 * sizeof(int));
	recieveing(conS, recEi, 2 * sizeof(int));
        memcpy(&recievedReq, recEi, sizeof(int));
	memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));
        if(recievedReq == closeCom){
            OFD_table[fd].local = -1;
            printf("%s closed\n",OFD_table[fd].filename);
            strcpy(OFD_table[fd].filename,"empty");
        }
    }
    else{
           printf("Error: file not opened\n");
        }
}

void nfs_fwrite(int fd, char *buf, int length){

    if(OFD_table[fd].local == fd){
           request = writeCom;
           size = length + 2*sizeof(int);
           elcurrentserver = (char *) malloc (12);
           header = (char *) malloc(2* sizeof(int));
           strcpy(elcurrentserver,OFD_table[fd].server);
           conS = mount_table[MTsearch(elcurrentserver)].mySocket;
           memcpy(header, &request, sizeof(int));
           memcpy(header+sizeof(int), &size, sizeof(int));
           sending(conS, header, 2*sizeof(int));
           sendData = (char *)malloc(length);
           memcpy(sendData, buf, length);
           sending(conS, sendData, length);
           free(sendData);
           sendData = (char *)malloc(sizeof(int));
           memcpy(sendData, &(OFD_table[fd].remote), sizeof(int));
            sending(conS, sendData, sizeof(int));
            free(sendData);
            sendData = (char *)malloc(sizeof(int));
            memcpy(sendData, &length, sizeof(int));
            sending(conS, sendData, sizeof(int));
            free(sendData);
            recEi = (char *) malloc(2 * sizeof(int));
            recieveing(conS, recEi, 2 * sizeof(int));
            memcpy(&recievedReq, recEi, sizeof(int));
            memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));
            if(recievedReq == writeCom){
		printf("%s successfully written to to\n",OFD_table[fd].filename);
            }
	}
	else{
		printf("Error writing to unopened file\n");
	}
}

void nfs_fread(int fd, char *buf, int length){


	if(OFD_table[fd].local == fd){
                request = readCom;
		size = 2*sizeof(int);
		elcurrentserver = (char *) malloc (12);
		header = (char *) malloc(2* sizeof(int));
		strcpy(elcurrentserver,OFD_table[fd].server);
		conS = mount_table[MTsearch(elcurrentserver)].mySocket;
		memcpy(header, &request, sizeof(int));
		memcpy(header+sizeof(int), &size, sizeof(int));
		sending(conS, header, 2*sizeof(int));
		sendData = (char *)malloc(sizeof(int));
		memcpy(sendData, &(OFD_table[fd].remote), sizeof(int));
		sending(conS, sendData, sizeof(int));
		free(sendData);
		sendData = (char *)malloc(sizeof(int));
		memcpy(sendData, &length, sizeof(int));
		sending(conS, sendData, sizeof(int));
		free(sendData);
                recEi = (char *) malloc(2 * sizeof(int));
		recieveing(conS, recEi, 2 * sizeof(int));
                memcpy(&recievedReq, recEi, sizeof(int));
		memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));
		if(recievedReq == readCom){
			recData = (char *) malloc(recievedSize);
			recieveing(conS, recData, recievedSize);
			buf = (char *) malloc(recievedSize);
			memcpy(buf, recData, recievedSize);
			buf[recievedSize] = 0;
			puts(buf);
		}

		
	}

	else{
		
            printf("Error:unopened file read\n");
	}
}

int nfs_remove(char *filesys, char *file){
	char *myName;
	myName = (char *)malloc(strlen(file));
	char *answerE;
	strcpy(myName,file);
	myServ = (char *)malloc(strlen(filesys));
        strcpy(myServ,filesys);
	if(MTsearch(myServ) == -1){
		printf("Error: %s not mounted\n", filesys);
	}
	else{
		if( ofdSearch(myServ,myName) == -1){
                    	request = remCom;
			size = strlen(myName);
			header = (char *) malloc(2* sizeof(int));
			conS = mount_table[MTsearch(filesys)].mySocket;
			memcpy(header, &request, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int));
			sending(conS, header, 2*sizeof(int));
			sendData = (char *)malloc(strlen(myName));
			strcpy(sendData,myName);

			sending(conS, sendData, strlen(sendData));

			recEi = (char *) malloc(2 * sizeof(int));
			recieveing(conS, recEi, 2 * sizeof(int));

			memcpy(&recievedReq, recEi, sizeof(int));
			memcpy(&recievedSize, recEi +sizeof(int), sizeof(int));

			if(recievedReq == remCom){

				recData = (char *) malloc(recievedSize);
				recieveing(conS, recData, recievedSize);
				answerE = (char *) malloc(recievedSize);

				memcpy(answerE, recData, recievedSize);

				if(strcmp("remove from server success\n",answerE) == 0){
					printf("%s ", myName);
				}

				puts(answerE);
				free(myName);
				free(recEi);
				free(sendData);
				free(header);
				free(myServ);
				return 0;
			}

			else{
				return -1;

			}

			
		}

		else{ 
                    printf("Error!!\n");

		}


	}
}


