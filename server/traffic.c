/******************************************************* 
	traffic.c
	c_network

	Created by John Paul Soliva on 6/19/16.
	Copyright © 2016 Soliva John Paul. All rights reserved.

	A simple server in the internet domain using TCP
	The port number is passed as an argument 
	This version runs forever, forking off a separate 
	process for each connection
 *******************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "GenericTypeDefs.h"		//Generic Type Definitions

#define TRAFFIC_SERVER_PORT	atoi(argv[1])	//Traffic handler Tcp port

static INT st_list[255];			//Socket list array
static INT st_i = 0;				//Socket list count
char uc_list[255];

/******** error() *********************
Exit when error occurs.
 *****************************************/
void error(const char *msg){
	perror(msg);		//do process error
	exit(1);			//exit
}

/******** printarray() *********************
Print array for debugging.
 *****************************************/
void printarray (int arg[], int length) {
	int n;
	for (n=0; n<length; ++n){
		printf("%d ", arg[n]);
	}
	printf("\n");
}

/******** main() *********************
Main loop.
 *****************************************/
int main(int argc, char *argv[])
{
    signal(SIGCHLD,SIG_IGN);
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n, a = 0;
	char buffer[255];

	memset(st_list, 0x00, 255);
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = TRAFFIC_SERVER_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while (1) {
		newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			 error("ERROR on accept");
//        printf("Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
		while(1){
			bzero(buffer,255);
			n = read(newsockfd,buffer,255);
			if (n < 0) error("ERROR reading from socket");
			if(strncmp(buffer, "logged", 6) == 0){

				strncpy(uc_list, &buffer[7], strlen(buffer) - 7);
				st_list[st_i] = atoi(uc_list);
//				printf("st_list: %d uc_list: %s\n", st_list[st_i], uc_list);
				st_i++;
				printf("(traffic_P) Child has updated client count to %d with its PID: %s\n", st_i, buffer);
				printf("(traffic_P) Child PID list:\n");
				printarray(st_list, st_i);

			}else if(strncmp(buffer, "get", 3) == 0){

				printf("(traffic_P) Child has write request on buffer\n");

//				bzero(buffer,256);
//				sprintf(buffer, "%d", st_i);
//				n = write(newsockfd, buffer, 256);
//				if (n < 0) error("ERROR writing to socket");

				n = write(newsockfd, uc_list, 256);
				if (n < 0) error("ERROR writing to socket");
				printf("(traffic_P) Done!\n");

			}else{

				if(n > 0){
					printf("(traffic) buffer is %s\n", buffer);
				}

			}
			if(n > 0){
				close(newsockfd);
				break;
			}
		}
//        close(newsockfd);
	 } /* end of while */
	 close(sockfd);
	 return 0; /* we never get here */
}