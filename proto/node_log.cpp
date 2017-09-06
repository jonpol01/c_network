/******************************************************* 
	renamed from lnx_server.cpp to node_log.cpp
	c_network

	Created by John Paul Soliva on 6/19/16.
	Copyright © 2016 Soliva John Paul. All rights reserved.

	A simple server in the internet domain using TCP
	The port number is passed as an argument 
	This version runs forever, forking off a separate 
	process for each connection

usage: bash$./<name>.o <port #>		ex: bash$./lnx_server.o 50000
 *******************************************************/

#include "net_lib.hpp"

/******** namespace() *********************
Namespace
*****************************************/
namespace Pathfinder
{
    
} //end namespace


/******** main() *********************
Main loop.
 *****************************************/
int main(int argc, char *argv[])
{
    using namespace std;

    Pathfinder::server serv;
    int n, a = 0;
    int sockfd, newsockfd, portno, pid;
    CHAR8 *ip_val;

    signal(SIGCHLD, SIG_IGN);
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* Clear st_list */
    memset(serv.st_list, 0x00, DATA_Page);

    /* Check if the user designated an open port */
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* Open a socket to bind & listen to */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        serv.error("ERROR opening socket"); 
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = MAIN_SERVER_PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        serv.error("ERROR on binding");
    listen(sockfd, 5);

    /* Accepting Client connections loop  */
    clilen = sizeof(cli_addr); //size of client address
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        fcntl(newsockfd, F_SETFL, O_NONBLOCK);
        if (newsockfd < 0)
            serv.error("ERROR on accept");
        serv.st_list[serv.st_i] = newsockfd;
        serv.st_i++;
        //		printf("(Parent)size of i and list : %d sock: %d\n", st_i, newsockfd);
        pid = fork(); //fork the socket to a seperate process
        if (pid < 0)
            serv.error("ERROR on fork");
        if (pid == 0)
        {
            ip_val = inet_ntoa(cli_addr.sin_addr);
            printf("(Parent)Client connected: Address: %s Pid: %d\n", ip_val, getpid() - 1);
            cout << "test" ;
            close(sockfd);
            serv.dostuff(newsockfd, ip_val);
            //			dostuff_cmd(newsockfd);
           exit(0);
        }
            //else close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}