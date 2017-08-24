/******************************************************* 
	lnx_server.cpp
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

/******** main() *********************
Main loop.
 *****************************************/
void main(int argc, char *argv[])
{

    server serv;
    int n, a = 0;
    signal(SIGCHLD, SIG_IGN);
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    n = 0;

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
//    if (sockfd < 0)
//        serv.error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = MAIN_SERVER_PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
//    if (bind(sockfd, (struct sockaddr *)&serv_addr,
//             sizeof(serv_addr)) < 0)
//        serv.error("ERROR on binding");
    listen(sockfd, 5);

    /* Accepting Client connections loop  */
    clilen = sizeof(cli_addr); //size of client address
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        fcntl(newsockfd, F_SETFL, O_NONBLOCK);
//        if (newsockfd < 0)
//            serv.error("ERROR on accept");
        serv.st_list[serv.st_i] = newsockfd;
        serv.st_i++;
        //		printf("(Parent)size of i and list : %d sock: %d\n", st_i, newsockfd);
        pid = fork(); //fork the socket to a seperate process
//        if (pid < 0)
//            serv.error("ERROR on fork");
        if (pid == 0)
        {
            printf("(Parent)Client connected: Address: %s Pid: %d\n", inet_ntoa(cli_addr.sin_addr), getpid() - 1);
            close(sockfd);
            serv.dostuff(newsockfd);
            //			dostuff_cmd(newsockfd);
            exit(0);
        }
        //else close(newsockfd);
    } /* end of while */
    close(sockfd);
    //return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void server::dostuff(const int sock)
{

    int n, a = 0;

    /* SEND a Logged in signal with its PID */
    //dostuff_traffic(CH_LOGGED);
    //error("got here!");
    /* Process Traffice loop */
    while (1)
    {

        bzero(c_Data, DATA_Page);          //clear buffer
        n = read(sock, c_Data, DATA_Page); //initiate read
                                           //		if (n < 0) error("ERROR reading from socket"); //exit when failed

        if (n > 0)
        {
            size_buff = strlen(c_Data); //size of string buffer
            printf("(Child %d) Here is the message: %s", sock, c_Data);
            n = write(sock, c_buff, size_buff);
//            if (n < 0)
//                error("ERROR writing to socket");
            //			dostuff_traffic(CH_WRITE);
        }

        //if(n < 0) dostuff_traffic(CH_READ);

        //		if (n < 0) error("ERROR writing to socket");
    }
}

} //end namespace
