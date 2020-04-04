#include "settings.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage();

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return -1;
    }
    // (1) create Server's socket
    int Csocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Client's socket, conn to Server

    if (Csocket < 0)
    {
        // is init failed

        printf("Failed Socket\n");
        exit(-1);
    }

    // (2) create Server's addr connect to
    struct sockaddr_in Saddr = {0};
    Saddr.sin_family = AF_INET;
    Saddr.sin_port = htons(CPORT);
    Saddr.sin_addr.s_addr = inet_addr(argv[1]);

    // (3) connect to Server
    if (connect(Csocket, (struct sockaddr *)&Saddr, sizeof(Saddr)) < 0)
    {
        // is conn failed

        printf("Failed connect\n");
        exit(-1);
    }

    /*
     * now conn success
     * this Client can send to Server
     */

    // (4) loop to send infomation
    int isExit = 0;
    byte recvBUFF[BUFF_SIZE];
    byte sendBUFF[BUFF_SIZE];
    while (!isExit && fgets(sendBUFF, BUFF_SIZE, stdin) != NULL)
    {
        isExit = strstr(sendBUFF, "\\exit");
        // if (isExit) *strstr(sendBUFF, "\\exit")='\0';

        write(Csocket, sendBUFF, strlen(sendBUFF));
    }

    // (5) send over, then close socket
    // write(Csocket, "\\exit", 5);
    close(Csocket);

    return 0;
}

void usage()
{
    printf("Usage: \n");
    printf("Client [ip_addr]\n");
}