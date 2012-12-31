
#include <sys/socket.h>       
#include <sys/types.h>        
#include <arpa/inet.h>       
#include <unistd.h>           
#include <stdlib.h>
#include <stdio.h>
#include "helper.h"          
#include "mksfs.h"	     

#define ecPort 2002
#define mLine  1000


                     
    struct sockaddr_in servaddr;  
    char buffer[mLine];
    char *endptr;               
    char *myLStable;
    char *header;
    char recEI[2*sizeof(int)];
    char *recData;
    char *DataSend;
    char openReq[20];
    char remReq[20];
    char *elWritBuf;
    char *elreadmos;
    long *LStableAdd;
    int lsCom = 1;
    int opCom = 2;
    int closeCom = 3;
    int writCom = 4;
    int readCom = 5;
    int remCom = 6;
    int listS;
    int connS;
    int port; 
    int request;
    int elremote;
    int isclose;
    int my_len;
    int size;
    int recvReq;
    int recvSize;
    int recvDATA;
    int remID = -1;
    


void connection_setup(){
	   
	    if ( (listS = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		    fprintf(stderr, "Error creating listening socket.\n");
		    exit(EXIT_FAILURE);
	    }

	    memset(&servaddr, 0, sizeof(servaddr));
	    servaddr.sin_family      = AF_INET;
	    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	    servaddr.sin_port        = htons(port);


	    if ( bind(listS, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
		    fprintf(stderr, "Error calling bind()\n");
		    exit(EXIT_FAILURE);
	    }

	    if ( listen(listS, LISTENQ) < 0 ) {
		    fprintf(stderr, "Error calling listening port()\n");
		    exit(EXIT_FAILURE);
	    }
    }

void fileSysOpen(){
	    mksfs(0);
    }
void fileSystemInit(){
	    mksfs(1);
    }


int Sending(int eSocket, char *ecStr, int len){
	int numSent;
	int mylen = len;
	numSent = send(eSocket, ecStr, mylen, 0);
	while(numSent != mylen){
		mylen = mylen - numSent;
		numSent = send(eSocket, ecStr, mylen, 0);
	}
	return 0;
}

int recieveing(int eSocket, char *ecStr, int len){

	int numreceived;
	int mylen = len;

	numreceived = recv(eSocket, ecStr, mylen, 0);
	while(numreceived != mylen){
		mylen = mylen - numreceived;
		numreceived = recv(eSocket, ecStr, mylen, 0);
	}
	return 0;
}




int main(int argc, char *argv[]) {
    
    if ( argc == 2 ) {
	port = strtol(argv[1], &endptr, 0);
	if ( *endptr ) {
	    fprintf(stderr, "Invalid port number.\n");
	    exit(EXIT_FAILURE);
	}
    }
    else if ( argc < 2 ) {
            port = ecPort;
    }

    else {
	fprintf(stderr, "Invalid arguments\n");
	exit(EXIT_FAILURE);
    }

    fileSystemInit();
    connection_setup();

    if ( (connS = accept(listS, NULL, NULL) ) < 0 ) {
	    fprintf(stderr, "Error!!\n");
	    exit(EXIT_FAILURE);
    }
    header = (char *)malloc(2 * sizeof(int));
    elreadmos = (char *)malloc(10000);
    myLStable = (char *)malloc(100);
    while ( 1 ) {
	recieveing(connS, recEI, 2 * sizeof(int));
	memcpy(&recvReq, recEI, sizeof(int));
	memcpy(&recvSize, recEI +sizeof(int), sizeof(int));
	switch (recvReq) {

		case 1:
			printf("root directory\n");
			LStableAdd = sfs_ls();
			strcpy(myLStable, LStableAdd);
			size = strlen(myLStable);
			DataSend = (char *)malloc(size);
			memcpy(header, &lsCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int)); 
			memcpy(DataSend, myLStable, size);
			Sending(connS, header, (2 * sizeof(int)));
			Sending(connS, DataSend, size);
			free(DataSend);
			break;
		case 2:
			printf("open file\n");
			recData = (char *)malloc(recvSize);
			recieveing(connS, recData, recvSize);
			memcpy(openReq, recData, recvSize);
			openReq[recvSize] = 0;
			remID = sfs_open(openReq);
			DataSend = (char *)malloc(sizeof(int));
			size = sizeof(int);
			memcpy(header, &opCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int));
			memcpy(DataSend, &remID, size);
			Sending(connS, header, (2 * sizeof(int)));
			Sending(connS, DataSend, size);
			free(recData);
			free(DataSend);
			break;
		case 3:
			printf("close file\n");
			recData = (char *)malloc(recvSize);
			recieveing(connS, recData, recvSize);
			memcpy(&elremote, recData, sizeof(int));
			sfs_close(elremote);
			memcpy(header, &closeCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int)); 
			Sending(connS, header, (2 * sizeof(int)));
                        free(recData);
			break;
		case 4:
			printf("write file\n");
			recData = (char *)malloc(recvSize);
			elWritBuf = (char *)malloc(recvSize);
			recieveing(connS, recData, recvSize - 8);
			memcpy(elWritBuf, recData, recvSize -8);
			elWritBuf[recvSize - 8] = 0;
			recieveing(connS, recData, sizeof(int));
			memcpy(&remID, recData, sizeof(int));
			recieveing(connS, recData, sizeof(int));
			memcpy(&my_len, recData, sizeof(int));
                        sfs_write(remID, elWritBuf, my_len);
			size = 0;
                        memcpy(header, &writCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int)); 
			Sending(connS, header, (2 * sizeof(int)));
                        free(recData);
			free(elWritBuf);
                        break;

		case 5:

			printf("read file\n");
			recData = (char *)malloc(recvSize);
                        recieveing(connS, recData, sizeof(int));
			memcpy(&remID, recData, sizeof(int));
			recieveing(connS, recData, sizeof(int));
			memcpy(&my_len, recData, sizeof(int));
                        sfs_read(remID, elreadmos, my_len);
                        size = strlen(elreadmos);
                        memcpy(header, &readCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int));
			Sending(connS, header, (2 * sizeof(int)));
                        DataSend = (char *)malloc(size);
			memcpy(DataSend, elreadmos, size);
			DataSend[size] = 0;
			puts(DataSend);
                        Sending(connS, DataSend, size);
                        break;

		case 6:
			printf("remove file\n");
			recData = (char *)malloc(recvSize);
                        recieveing(connS, recData, recvSize);
			memcpy(remReq, recData, recvSize);
			remReq[recvSize] = 0;
			printf("file to remove = %s\n",remReq);
                        if (sfs_remove(remReq) == -1){
				size = strlen("Error: file doesnot exist\n");
				DataSend = (char *)malloc(size);
				strcpy(DataSend, "Error: file doesnt exist\n");
			}

			else{
				size = strlen("file removed successfully\n");
				DataSend = (char *)malloc(size);
				strcpy(DataSend, "file remove successfull\n");

			}
                        memcpy(header, &remCom, sizeof(int));
			memcpy(header+sizeof(int), &size, sizeof(int)); 
			Sending(connS, header, (2 * sizeof(int)));
                        Sending(connS, DataSend, size);
                        free(recData);
			free(DataSend);
                        break;
               default:
			printf("Invalid input\n");
                        break;
	}

    }

}



