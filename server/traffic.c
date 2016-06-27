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

static UINT st_list[255];			//Socket list array
static UINT st_count = 0;
static UINT st_i = 0;				//Socket list count
BYTE uc_list[256];

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
	char buffer[256];

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
		if (newsockfd < 0)
			 error("ERROR on accept");
//        printf("Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
		while(1){
			bzero(buffer,256);
			n = read(newsockfd,buffer,255);
			if (n < 0) error("ERROR reading from socket");
			if(strncmp(buffer, "logged", 6) == 0){

				st_i++;
				printf("(traffic_P) Child has updated client count to %d\n", st_i);

				bzero(buffer,256);
				n = read(newsockfd,buffer,255);
				if (n < 0) error("ERROR reading from socket");

				memset(st_list, 0x00, 255);
				bzero(uc_list,256);
				printf("(traffic_P) Child has sent Client list: ");
				for(a = 0; a < strlen(buffer); a++){
					st_list[a] = (int)(buffer[a] - '0');
					uc_list[a] = buffer[a];
					printf("%c ", uc_list[a]);
				}
				printf("!\n");
				printarray(st_list, strlen(buffer));

			}else if(strncmp(buffer, "data", 3) == 0){

				printf("(traffic_P) Child has write request on buffer\n");

//				bzero(buffer,256);
//				sprintf(buffer, "%d", st_i);
//				n = write(newsockfd, buffer, 256);
//				if (n < 0) error("ERROR writing to socket");

				n = write(newsockfd, uc_list, 256);
				if (n < 0) error("ERROR writing to socket");
				printf("(traffic_P) Done!\n");

			}else{

//				memset(st_list, 0x00, 255);
//				bzero(uc_list,256);
//				printf("(traffic_P) Child has sent Client list: ");
//				for(a = 0; a < strlen(buffer); a++){
//					st_list[a] = (int)(buffer[a] - '0');
//					uc_list[a] = buffer[a];
//					printf("%c ", uc_list[a]);
//				}
//				printf("!\n");
//				printarray(st_list, strlen(buffer));

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