/******************************************************* 
	traffic.cpp
	c_network

	Created by John Paul Soliva on 6/19/16.
	Copyright © 2016 Soliva John Paul. All rights reserved.

	A simple server in the internet domain using TCP
	The port number is passed as an argument 
	This version runs forever

usage: bash$./<name>.o <port #>		ex: bash$./traffic.o 50010
 *******************************************************/
#include <iostream>
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

#define TRAFFIC_SERVER_PORT		(atoi(argv[1]))	//Traffic handler Tcp port
PRIVATE INT st_Pid_list[255];			//Socket list array
PRIVATE INT st_PidData_list[255][2];		//Socket list array
PRIVATE INT st_i = 0;				//Socket list count
CHAR8 c_Data[255];					//Client to Server data buffer
CHAR8 c_buff[255];					//local buffer
CHAR8 c_Pid_list[255];

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
//	char buffer[255];
//	char buff_check[255];

	memset(st_Pid_list, 0x00, 255);
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
			bzero(c_buff,255);
			n = read(newsockfd,c_buff,255);
			if (n < 0) error("ERROR reading from socket");
			if(strncmp(c_buff, "logged", 6) == 0){

//				memcpy(&st_Pid_list[st_i], atoi(&buffer[7]), strlen(buffer) - 7);
				strncpy(&c_Pid_list[st_i], &c_buff[7], strlen(c_buff) - 7);
//				strncpy(c_Pid_list, &buffer[7], strlen(buffer) - 7);
				st_Pid_list[st_i] = atoi(&c_Pid_list[st_i]);
				st_i++;
				printf("(traffic_P) Child has updated client count to %d with its PID: %s\n", st_i, c_buff);
				printf("(traffic_P) Child PID list:\n");
				printarray(st_Pid_list, st_i);

			}else if(strncmp(c_buff, "get", 3) == 0){

				printf("(traffic_P) Child has write request on buffer\n");

				n = write(newsockfd, c_Pid_list, 256);
				if (n < 0) error("ERROR writing to socket");
				printf("(traffic_P) Done!\n");

			}else if(strncmp(c_buff, "write", 5) == 0){

				int		i;
				bool	i_Pid_match = false;
				char	c_buff_Pid[255];
				char	c_buff_Pid_Chk[255];
				size_t	size_buff = strlen(c_buff);
				size_t	size_Pid;
				size_t	size_Data;
				
				//printf("(traffic_P) DATA: %s len: %ld\n", c_buff, size_buff);
				bzero(c_buff_Pid_Chk,255);
				strncpy(c_buff_Pid_Chk, &c_buff[6], (size_buff - 7));
				for(i = 0; i < st_i; i++){
					sprintf(c_buff_Pid, "%d", st_Pid_list[i]);
					size_Pid = strlen(c_buff_Pid);
					if(strncmp(c_buff_Pid_Chk, c_buff_Pid, size_Pid) == 0){
						size_Data = size_buff - (6 + strlen(c_buff_Pid));
						strncpy(c_Data, &c_buff[7 + strlen(c_buff_Pid)], size_Data);
						printf("(traffic_P) PID Matched found %s to %s Data: %s\n", c_buff_Pid, c_buff_Pid_Chk, c_Data);
//						sprintf(st_PidData_list,, c_buff_Pid, c_Data);
						i_Pid_match = true;
						bzero(c_Data,255);
						break;
					}else{
						//printf("(traffic_P) No PID Matched on the list %s to %s\n", c_buff_Pid, c_buff_Pid_Chk);
						i_Pid_match = false;
					}
				}

				if(i_Pid_match >= 1){
					printf("(traffic_P) PID match Flag is %d and i: %d\n", i_Pid_match, i);
				}
//				sprintf(c_buff, "%c", (char)st_Pid_list[st_i]);
//				size_t	size_buff = strlen(c_buff);
//				printf("c_buff: %c\n", c_buff);
//				if(strncmp(&c_buff[6], c_buff, size_buff) == 0){
//					printf("pid list: %s buffer: %s\n", c_buff, &c_buff[6]);
//				}else{
//					printf("pid list: %s buffer: %s\n", c_buff, &c_buff[6]);
//					error("error cmp\n");
//				}

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