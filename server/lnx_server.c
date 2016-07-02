/******************************************************* 
	lnx_server.c
	c_network

	Created by John Paul Soliva on 6/19/16.
	Copyright © 2016 Soliva John Paul. All rights reserved.

	A simple server in the internet domain using TCP
	The port number is passed as an argument 
	This version runs forever, forking off a separate 
	process for each connection

usage: bash$./<name>.o <port #>		ex: bash$./lnx_server.o 50000
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

#define MAIN_SERVER_PORT	(atoi(argv[1]))	//Our Main server Tcp Port
#define TRAFFIC_SERVER_PORT	50010	//Traffic handler Tcp port
#define RELAY_SERVER_PORT	50020	//Relay server Tcp Port
#define CH_LOGGED		1
#define CH_REQUEST		2
#define CH_PID			3

PRIVATE UINT st_list[255];			//Socket list array
PRIVATE UINT st_i = 0;				//Socket list count
CHAR8 c_Data[255];					//Client to Child Server data buffer
CHAR8 c_buff[1024];					//local buffer
INT n, a = 0;						//int worker

size_t size_buff;					//size of data buffer

void dostuff(INT);			//function prototype
void dostuff_cmd(INT);		//function Cmd
void dostuff_traffic(INT,INT);	//function Update Client count
void dostuff_Relay(INT);	//function

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
void printarray (INT arg[], INT length) {
	for (n=0; n<length; ++n){
		printf("%d ", arg[n]);
	}
	printf("\n");
}

/******** main() *********************
Main loop.
 *****************************************/
int main(INT argc, CHAR8 *argv[]){

	signal(SIGCHLD,SIG_IGN);
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	n = 0;

	/* Clear st_list */
	memset(st_list, 0x00, 255);

	/* Check if the user designated an open port */
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	/* Open a socket to bind & listen to */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = MAIN_SERVER_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);

	/* Accepting Client connections loop  */
	clilen = sizeof(cli_addr);	//size of client address
	while (1) {
		newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		st_list[st_i] = newsockfd;
		st_i++;
//		printf("(Parent)size of i and list : %d sock: %d\n", st_i, newsockfd);
		pid = fork();	//fork the socket to a seperate process
		if (pid < 0)
			error("ERROR on fork");
		if (pid == 0) {
//			printf("(Parent)Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), getpid());
			close(sockfd);
			dostuff(newsockfd);
			exit(0);
		}
		//else close(newsockfd);
	} /* end of while */
	close(sockfd);
	return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock){

	n = 0;
	a = 0;

	/* SEND a Logged in signal with its PID */
	dostuff_traffic(CH_LOGGED, sock);

	/* Process Traffice loop */
	while(1){

		bzero(c_Data, 255);	//clear buffer
		n = read(sock, c_Data, 255);	//initiate read
		if (n < 0) error("ERROR reading from socket"); //exit when failed

		if(n > 0){
			size_buff = strlen(c_Data);	//size of string buffer
			printf("(Child %d) Here is the message: %s\n", sock, c_Data);
			dostuff_traffic(3, sock);

			//sprintf(c_buff, "%d,%s", sock, c_Data);
			//if(size_buff > 0){
			//	for(a = 0; a <= size_buff; a++){
//					n = write(st_list[a], buffer, 256);
			//	}
			//}
		}

		if (n < 0) error("ERROR writing to socket");
	}
}

/******** dostuff_traffic() *********************
Client to Traffic server handler.
 *****************************************/
void dostuff_traffic (int proc, int id){

	n = 0;
	a = 0;
	int sockfd, newsockfd, portno;
	int len = 0;
	struct sockaddr_in serv_addr, cli_addr;

	/* Setup a socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = TRAFFIC_SERVER_PORT; //tcp port 50010
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(portno);

	/* connect with server's socket */
	int res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	/* If failed we exit the function */
	if(res == -1){
		exit(1);
		error("failed");
	};

	/* Client has logged in */
	if(proc == 1){

		/* tell the traffic server that we logged in together with its PID */
		bzero(c_buff,1024);
		size_buff = sprintf(c_buff, "logged,%d", getpid());
		n = write(sockfd, c_buff, size_buff);
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	/* Client request data */
	}else if(proc == 2){

		/* Write request first */
		bzero(c_buff,1024);
		size_buff = sprintf(c_buff, "get");
		n = write(sockfd, c_buff, size_buff);
		if (n < 0) error("ERROR writing to socket");

		/* Write data to server buffer */
		bzero(c_buff,1024);
		size_buff = sprintf(c_buff, "%d", id);
		n = write(sockfd, c_buff, size_buff);
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	/* Client write data */
	}else if(proc == 3){

		/* Write data */
		bzero(c_buff,1024);
		size_buff = sprintf(c_buff, "write,%d!%s", getpid(), c_Data);
		n = write(sockfd, c_buff, size_buff);
		if (n < 0) error("ERROR writing to socket");
		bzero(c_Data,255);

		/* Close the socket */
		close(sockfd);
		return;

	}
}

void dostuff_Relay (int proc){

	n = 0;
	a = 0;
	int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr, cli_addr;

	/* Setup a socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = RELAY_SERVER_PORT; //tcp port 50010
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(portno);

	/* connect with server's socket */
	int res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	/* If failed we exit the function */
	if(res == -1){
		exit(1);
	};

	if(proc == 1){

	}
}

void dostuff_cmd (INT sock){

	n = 0;
	a = 0;

	while(1){
		bzero(c_buff,256);
		n = read(sock,c_buff,255);
		if (n < 0) error("ERROR reading from socket");
		printf("Here is the message: %s\n",c_buff);
		if(n >= 0){
			n = write(sock,"cmd$> ",6);
			if(st_i > 1){
				for(a = 0; a < st_i; a++){
				   n = write(st_list[a], c_buff, 256);
				}
			}
		}
		if (n < 0) error("ERROR writing to socket");
	}
}
