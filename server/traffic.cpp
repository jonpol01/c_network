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
#include <fcntl.h>
#include "GenericTypeDefs.h"		//Generic Type Definitions

#define TRAFFIC_SERVER_PORT				(atoi(argv[1]))	//Traffic handler Tcp port
#define BUFF_PID_Pos					6
#define BUFF_DATA_Pos					(7 + strlen(c_buff_Pid))
#define	DATA_Page						256
#define	LIST_PID_POS					0
#define	LIST_DATA_POS					1

PRIVATE INT st_Pid_list[DATA_Page];			//Socket list array
PRIVATE INT st_i = 0;				//Socket list count
INT n, a = 0;
CHAR8 c_PidData_list[DATA_Page*4][DATA_Page*4][DATA_Page];		//Socket Data on [PID][Data] Array
CHAR8 c_Pid_list[DATA_Page];
CHAR8 c_Data[DATA_Page];					//Client to Server data buffer
CHAR8 c_buff[DATA_Page];					//local buffer

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
	bool	i_Pid_match = false;

	memset(st_Pid_list, 0x00, DATA_Page);
	memset(c_Pid_list, 0x00, DATA_Page);
	memset(c_Data, 0x00, DATA_Page);
	memset(c_buff, 0x00, DATA_Page);

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
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		fcntl(newsockfd, F_SETFL, O_NONBLOCK);

		if (newsockfd < 0)
			 error("ERROR on accept");
        //printf("Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
		while(1){
			bzero(c_buff,DATA_Page);
			n = read(newsockfd,c_buff,DATA_Page);
//			if (n < 0) error("ERROR reading from socket");

			if(n < 0){
				printf("test\n");

				int i;
//				if(i_Pid_match == true){
					bzero(c_buff,DATA_Page);
					for(i = 0; i < st_i; i++){
						sprintf(c_buff,"%s,%s", &c_PidData_list[i][LIST_PID_POS][0], &c_PidData_list[i][LIST_DATA_POS][0]);
						printf("(traffic_P) send buffer: %s st_i: %d\n", c_buff, st_i);				
						n = write(newsockfd, c_buff, DATA_Page);
						if (n < 0) error("ERROR writing to socket");
					}
					bzero(c_buff,DATA_Page);
					i_Pid_match = false;
//				}
			}

			if(strncmp(c_buff, "logged", 6) == 0){

				strncpy(&c_Pid_list[st_i], &c_buff[7], strlen(c_buff) - 7);
				st_Pid_list[st_i] = atoi(&c_Pid_list[st_i]);
				st_i++;
				printf("(traffic_P) Child has updated client count to %d with its PID: %s\n", st_i, c_buff);
				printf("(traffic_P) Child PID list:\n");
				printarray(st_Pid_list, st_i);

			}else if(strncmp(c_buff, "write", 5) == 0 && n > 0){

				int		i;
				char	c_buff_Pid[DATA_Page];
				char	c_buff_Pid_Chk[DATA_Page];
				size_t	size_buff = strlen(c_buff) - 1;
				size_t	size_Pid;
				size_t	size_Data;

				printf("(traffic_P) DATA: %s len: %ld\n", c_buff, size_buff);
				bzero(c_buff_Pid_Chk,DATA_Page);
				strncpy(c_buff_Pid_Chk, &c_buff[6], (size_buff - BUFF_PID_Pos));
				for(i = 0; i < st_i; i++){
					sprintf(c_buff_Pid, "%d", st_Pid_list[i]);
					size_Pid = strlen(c_buff_Pid);
					if(strncmp(c_buff_Pid_Chk, c_buff_Pid, size_Pid) == 0){
						size_Data = size_buff - BUFF_DATA_Pos;
						strncpy(c_Data, &c_buff[7 + strlen(c_buff_Pid)], size_Data);
						sprintf(&c_PidData_list[i][0][0], "%s", c_buff_Pid);
						sprintf(&c_PidData_list[i][1][0], "%s", c_Data);
						i_Pid_match = true;
						bzero(c_buff_Pid,DATA_Page);
						bzero(c_Data,DATA_Page);
						break;
					}else{
						i_Pid_match = false;
					}
				}

				if(i_Pid_match >= 1){
//					printf("(traffic_P) PID match Flag is %d and i: %d\n PID: %s data: %s\n", i_Pid_match, i, &c_PidData_list[i][0][0], &c_PidData_list[i][1][0]);
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
//			}else if(strncmp(c_buff, "get", 3) == 0){
			}
			
			if(n > 0){
//				close(newsockfd);
				break;
			}
		}
//        close(newsockfd);
	 } /* end of while */
	 close(sockfd);
	 return 0; /* we never get here */
}