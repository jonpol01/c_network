#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include "GenericTypeDefs.h" //Generic Type Definitions

#define MAIN_SERVER_PORT (atoi(argv[1])) //Our Main server Tcp Port
#define TRAFFIC_SERVER_PORT 50010        //Traffic handler Tcp port
#define RELAY_SERVER_PORT 50020          //Relay server Tcp Port
#define CH_LOGGED 1
#define CH_READ 2
#define CH_WRITE 3
#define DATA_Page 256

namespace Pathfinder
{
using namespace std;

class server
{
  public:
    uint st_list[DATA_Page];    //Socket list array
    uint st_i;                  //Socket list count
    CHAR8 c_Data[DATA_Page];     //Client to Child Server data buffer
    CHAR8 c_buff[DATA_Page * 4]; //local buffer

    size_t size_buff; //size of data buffer

    void dostuff(const uint16_t, const CHAR8 *); //function prototype
    void dostuff_cmd(const int8_t);           //function Cmd
    void dostuff_traffic(const int8_t);       //function Update Client count
    void dostuff_Relay(const int8_t);         //function

    void error(const CHAR8 *);
    void cl_off(const CHAR8 *);
    int8_t p_read(const int8_t, const CHAR8 *);
    void p_write(const int8_t, const CHAR8 *);

    void printarray(const uint8_t, const uint8_t);
  private:
};

/******** error() **********************************
*Exit when error occurs.
***************************************************/
void server::error(const CHAR8 *msg)
{
    perror(msg); //do process error
    exit(1);     //exit
}

/******** cl_off() *********************************
*Exit when client disconnect.
***************************************************/
void server::cl_off(const CHAR8 *msg)
{
    cout << "(Client:" << msg << ") has disconnected.\n"; //do process error
    exit(1); //exit
}

/******** printarray() *****************************
*Print array for debugging.
***************************************************/
void printarray (const uint8_t arg[], const int8_t length) {
    uint8_t n;
    for (n=0; n<length; ++n){
        cout << arg[n] << endl;
    }
    cout << "\n";
}

/******** p_read() & p_write() *********************
* Basic Data transmission
* Child function.., this runs after a fork() 
***************************************************/
int8_t server::p_read(const int8_t sockfd, const CHAR8 *ip)
{
    int8_t n = 0;
    bzero(c_buff, DATA_Page * 4);
    n = read(sockfd, c_buff, DATA_Page * 4);
    if (n > 0)
    {
        size_buff = strlen(c_buff); //size of string buffer
        cout << "(Client: " << ip << ") " << "debug message: " << c_buff;
    }
    return(n);
}

void server::p_write(const int8_t sockfd, const CHAR8 *data)
{
    int8_t n = 0;
    bzero(c_buff, DATA_Page * 4);
    size_buff = sprintf(c_buff, "%s", data);
    n = write(sockfd, c_buff, size_buff);
    //    if (n < 0)
    //error("ERROR writing to socket");
}

/******** DOSTUFF() ********************************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 **************************************************/
void server::dostuff(const uint16_t sock, const CHAR8 *ip)
{
    int n, a = 0;
    /* Process Traffice loop */
    while (1)
    {
        bzero(c_buff, DATA_Page);          //clear buffer
        n = p_read(sock, ip);
        if (n == 0)
        {
            cl_off(ip); //exit when failed
        }
    }
}

/******** dostuff_traffic() *********************
*Client to Traffic server handler.
*****************************************/
void server::dostuff_traffic(const int8_t proc)
{
    int n, a = 0;
    int sockfd, newsockfd, portno;
    int len = 0;
    struct sockaddr_in serv_addr, cli_addr;

    /* Setup a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //    if (sockfd < 0)
    //error("child: ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = TRAFFIC_SERVER_PORT; //tcp port 50010
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(portno);

    /* connect with server's socket */
    int res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    /* If failed we exit the function */
    if (res == -1)
    {
        exit(1);
        //error("failed");
    };

    /* Client has logged in */
    if (proc == 1)
    {
        /* tell the traffic server that we logged in together with its PID */
        bzero(c_buff, 1024);
        size_buff = sprintf(c_buff, "logged,%d", getpid());
        n = write(sockfd, c_buff, size_buff);
        //       if (n < 0)
        //error("ERROR writing to socket");
        /* Close the socket */
        //close(sockfd);
        return;
        /* Client request data */
    }
    else if (proc == 2)
    {
        /* Write request first */
        //write(sockfd, "get");
        /* Read data to server buffer */
        while (true)
        {
            //read(sockfd);
            bzero(c_buff, DATA_Page * 4);
            n = read(sockfd, c_buff, DATA_Page * 4);
            //         if (n < 0)
            //error("ERROR writing to socket");
            if (strlen(c_buff) <= 1)
            {
                break;
            }
            else
            {
                printf("REQUEST: %s\n", c_buff);
            }
        }
        /* Close the socket */
        //close(sockfd);
        return;
        /* Client write data */
    }
    else if (proc == 3)
    {
        /* Write data */
        bzero(c_buff, DATA_Page * 4);
        size_buff = sprintf(c_buff, "write,%d!%s", getpid(), c_Data);
        n = write(sockfd, c_buff, size_buff);
        //      if (n < 0)
        //error("ERROR writing to socket");
        bzero(c_Data, DATA_Page);

        /* Close the socket */
        //close(sockfd);
        return;
    }
}

/*
void server::dostuff_cmd(const INT sock)
{

    int n, a = 0;

    while (1)
    {
        bzero(c_buff, DATA_Page);
        n = read(sock, c_buff, DATA_Page);
        if (n < 0)
            error("ERROR reading from socket");
        printf("Here is the message: %s\n", c_buff);
        if (n >= 0)
        {
            n = write(sock, "cmd$> ", 6);
            if (st_i > 1)
            {
                for (a = 0; a < st_i; a++)
                {
                    n = write(st_list[a], c_buff, DATA_Page);
                }
            }
        }
        if (n < 0)
            error("ERROR writing to socket");
    }
}
*/

} //end namespace