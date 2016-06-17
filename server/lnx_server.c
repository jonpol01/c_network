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

void dostuff(int); /* function prototype */
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
         printarray(list, i);
         printf("size of i and list : %d\n", i);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             printf("Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), newsockfd);
             n = write(newsockfd,"cmd$> ",6);
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