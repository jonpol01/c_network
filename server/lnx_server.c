/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
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

#define MAIN_SERVER_PORT	50000	//Our Main server Tcp Port
#define TRAFFIC_SERVER_PORT 50010	//Traffic handler Tcp port
#define RELAY_SERVER_PORT	50020	//Relay server Tcp Port
#define CH_LOGGED		1
#define CH_REQUEST		2
#define CH_CONTROL		3

PRIVATE UINT st_list[255];			//Socket list array
PRIVATE UINT st_i = 0;				//Socket list count
PRIVATE UINT st_count = 0;
BYTE c_Data[256];					//Client to Child data buffer
BYTE c_buff[256];					//Client to Child data buffer
size_t size_buff;					//size of data buffer

void dostuff(int);			//function prototype
void dostuff_cmd(int);		//function Cmd
void dostuff_traffic(int);	//function Update Client count
void dostuff_Relay(int);	//function

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
int main(int argc, char *argv[]){

	signal(SIGCHLD,SIG_IGN);
	UINT sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	UINT n = 0;

	memset(st_list, 0x00, 255);
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
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
		st_list[st_i] = newsockfd;
		st_i++;
		printf("(Parent)size of i and list : %d\n", st_i);
		if (newsockfd < 0)
			error("ERROR on accept");
		pid = fork();
		if (pid < 0)
			error("ERROR on fork");
		if (pid == 0)  {
			printf("(Parent)Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
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

	int n, a = 0;

	/* SEND a Logged in signal */
	dostuff_traffic(CH_LOGGED);

	/* Process Traffice loop */
	while(1){

		bzero(c_Data, 256);	//clear buffer
		n = read(sock, c_Data, 255);	//initiate read
		if (n < 0) error("ERROR reading from socket"); //exit when failed
		size_buff = strlen(c_Data);	//size of string buffer
		printf("(Child %d) Here is the message: %s\n", sock, c_Data);

		if(n > 0){
			if(size_buff > 0){
				for(a = 0; a <= size_buff; a++){
//					n = write(st_list[a], buffer, 256);
				}
			}
		}

		if (n < 0) error("ERROR writing to socket");
	}
}

void dostuff_cmd (int sock){

	int n, a = 0;

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

void dostuff_traffic (int proc){

	int n, a = 0;
	char buffer[256];
	int sockfd, newsockfd, portno;
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
	};

	if(proc == 1){

		/* tell the traffic server that we logged in */
		bzero(buffer,256);
		sprintf(buffer, "logged");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Write & update his Client list */
		bzero(buffer,256);
		for(a = 0; a < st_i; a++){
			snprintf(&buffer[a], st_i+1, "%d", st_list[a]);
//			printf("%c ", buffer[a]);
		}
//		printf("\n");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	}else if(proc == 2){

		/* Write request first */
		bzero(buffer,256);
		sprintf(buffer, "data");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Write data to server buffer */
		bzero(buffer,256);
//		sprintf(buffer, "data");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Then, read the incoming client count */
//		bzero(buffer,256);
//		n = read(sockfd,buffer,255);
//		if (n < 0) error("ERROR reading from socket");
//		st_count = atoi(buffer);
//		printf("(Child) Client count is %d\n", st_count);

		bzero(buffer,256);
		memset(st_list, 0x00, 255);
		n = read(sockfd,buffer,255);
		if (n < 0) error("ERROR reading from socket");

		size_buff = strlen(buffer);
//		printf("(Child) Child has recv new Client list: ");
		for(a = 0; a < strlen(buffer); a++){
			st_list[a] = (int)(buffer[a] - '0');
//			printf("%d ", st_list[a]);
//			printf("%c ", buffer[a]);
		}
//		printf("\n");

		/* Close the socket */
		close(sockfd);
		return;

	}else if(proc == 3){

		/* Write Client list */
		bzero(buffer,256);
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	}

}

void dostuff_Relay (int proc){

	int n, a = 0;
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

		/* Write request first */
		bzero(c_buff,256);
		sprintf(c_buff, "data");
		n = write(sockfd, c_buff, strlen(c_buff));
		if (n < 0) error("ERROR writing to socket");

		/* Write data to server buffer */
//		sprintf(buffer, "data");
		n = write(sockfd, c_Data, strlen(c_Data));
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	}
}