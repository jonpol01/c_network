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

//static int list = [];
static int list[255];
static int i = 0;

void dostuff(int);      /* function prototype */
void dostuff_cmd(int);  /* function Cmd */
void dostuff_traffic(int);  /* function Update Client count */
void error(const char *msg)
{
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

int main(int argc, char *argv[])
{
     signal(SIGCHLD,SIG_IGN);
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n = 0;
     //memset(list, 0x00, 255);
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
         list[i] = newsockfd;
         i++;
//         printarray(list, i);
         printf("(Parent)size of i and list : %d\n", i);
         if (newsockfd < 0) 
            error("ERROR on accept");
         pid = fork();
         if (pid < 0)
            error("ERROR on fork");
         if (pid == 0)  {
             printf("(Parent)Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
//             n = write(newsockfd,"cmd$> ",6);
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
void dostuff (int sock)
{
   int n;
   int a = 0;
   char buffer[256];

   dostuff_traffic(1);
   while(1){
       bzero(buffer,256);
       n = read(sock,buffer,255);
       if (n < 0) error("ERROR reading from socket");
       printf("Here is the message: %s\n",buffer);
       if(n >= 0){
           //n = write(sock,"cmd$> ",6);
           if(i > 1){
               for(a = 0; a < i; a++){
                   n = write(list[a], buffer, 256);
                }
            }
        }
        if (n < 0) error("ERROR writing to socket");
   }
}

void dostuff_cmd (int sock)
{
    int n;
    int a = 0;
    char buffer[256];

    while(1){
        bzero(buffer,256);
        n = read(sock,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);
        if(n >= 0){
            n = write(sock,"cmd$> ",6);
	        if(i > 1){
    	        for(a = 0; a < i; a++){
        	        n = write(list[a], buffer, 256);
            	}
        	}
    	}
    	if (n < 0) error("ERROR writing to socket");
	}
}

void dostuff_traffic (int proc)
{
    int n;
    int a = 0;
    char buffer[256];
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;

    /* Setup a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 50010; //tcp port 50010
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

        /* Close the socket */
	    close(sockfd);
	    return;
    }else if(proc == 2){

	    /* Write Get request first */
        bzero(buffer,256);
        sprintf(buffer, "get");
        n = write(sockfd, buffer, strlen(buffer));

        /* Then, read the incoming client count */
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");

        
        /* Close the socket */
	    close(sockfd);
	    return;
    }

}