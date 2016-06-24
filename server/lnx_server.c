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

#define DATA_SERVER_PORT 50000
#define TRAFFIC_SERVER_PORT 50010

static int st_list[255];
static int st_i = 0;
static int st_count = 0;

void dostuff(int);      /* function prototype */
void dostuff_cmd(int);  /* function Cmd */
void dostuff_traffic(int);  /* function Update Client count */
void error(const char *msg){
	perror(msg);
	exit(1);
}

void printarray (int arg[], int length) {
	int n;
	for (n=0; n<length; ++n){
		printf("%d ", arg[n]);
	}
	printf("\n");
}

int main(int argc, char *argv[]){
	 signal(SIGCHLD,SIG_IGN);
	 int sockfd, newsockfd, portno, pid;
	 socklen_t clilen;
	 struct sockaddr_in serv_addr, cli_addr;
	 int n = 0;

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
	char buffer[256];

	dostuff_traffic(1);
	dostuff_traffic(3);

	while(1){
		dostuff_traffic(2);
		printf("(Child %d) st_count: %d\n",sock,st_count);
		bzero(buffer,256);
		n = read(sock,buffer,255);
		if (n < 0) error("ERROR reading from socket");
		printf("(Child %d) Here is the message: %s\n",sock,buffer);
		if(n >= 0){
		   //n = write(sock,"cmd$> ",6);
			if(st_count > 1){
				for(a = 0; a < st_count; a++){
//				   if(st_list[a] != st_list[st_i-1]) n = write(st_list[a], buffer, 256);
					n = write(st_list[a], buffer, 256);
				}
			}
		}
		if (n < 0) error("ERROR writing to socket");
	}
}

void dostuff_cmd (int sock){

	int n, a = 0;
	char buffer[256];

	while(1){
		bzero(buffer,256);
		n = read(sock,buffer,255);
		if (n < 0) error("ERROR reading from socket");
		printf("Here is the message: %s\n",buffer);
		if(n >= 0){
			n = write(sock,"cmd$> ",6);
			if(st_i > 1){
				for(a = 0; a < st_i; a++){
				   n = write(st_list[a], buffer, 256);
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

		/* Close the socket */
		close(sockfd);
		return;

	}else if(proc == 2){

		/* Write Get request first */
		bzero(buffer,256);
		sprintf(buffer, "get");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Then, read the incoming client count */
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) error("ERROR reading from socket");
		st_count = atoi(buffer);
		printf("(Child) Client count is %d\n", st_count);

		/* Close the socket */
		close(sockfd);
		return;

	}else if(proc == 3){

		/* Write Client list */
		bzero(buffer,256);
		for(a = 0; a < st_i; a++){
			snprintf(&buffer[a], st_i+1, "%d", st_list[a]);
			//sprintf(&buffer[a], "%d", st_list[a]);
			printf("%c ", buffer[a]);
		}
		printf("\n");
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");

		/* Close the socket */
		close(sockfd);
		return;

	}

}