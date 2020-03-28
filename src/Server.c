#include "settings.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    // (1) create Server's socket
    int Ssocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // Server's socket, listening port
    
    if (Ssocket < 0){
        // is init failed

        printf("Failed Socket\n");
        exit(-1);
    }

    // (2) create Server's sockaddr
    struct sockaddr_in Saddr = {0};
    Saddr.sin_family = AF_INET;
    // Saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    Saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    Saddr.sin_port = htons(SPORT);

    // (3) bind Server's Socket & Port
    if ( bind(Ssocket, (struct sockaddr*)&Saddr, sizeof(Saddr)) < 0){
        // is bind failed

        printf("Failed bind\n");
        exit(-1);
    }

    // (4) set socket listning
    if ( listen(Ssocket, BACKLOG) < 0){
        // is sleep

        printf("Failed listen\n");
        exit(-1);
    }
    else{
        printf("Now working...\n");
    }

    /*
     *  now Sever could listen, and accept from client
     */

    byte buff[BUFF_SIZE];   // buffer save the accept
    int  isExit = 0;        // is user want to exit process
    int Csocket;            // socket from Client by connet/accept
    struct sockaddr_in Caddr = {0};
                            // sockaddr from Client
                            // read data form here

    FILE* flog = fopen(LISTEN_LOG, "wb");
    while(!isExit){
        /*
         * loop unitl exit cmd
         * accept from client, read data and write out
         */

        socklen_t lenCaddr = sizeof(Caddr);
        Csocket = accept(Ssocket, (struct sockaddr*)&Caddr, &lenCaddr);
        if (Csocket < 0){
            // is accept failed
            
            printf("Faild accept");
            // exit(-1);
            continue;
        }
        
        // success accept from Client
        close(Ssocket);
        ssize_t readSize;
        while( (readSize = read(Csocket, buff, BUFF_SIZE)) > 0){
            // loop unitl buff read over

            if (isExit = (strstr(buff, "\\exit")!=NULL) ) break;

            // write(Csocket, buff, readSize);
            printf("%s", buff);
            for (byte *ptmp = buff; ptmp-buff < readSize; ptmp++){
                fputc(*ptmp, flog);
                putchar(*ptmp);
            }

            fputc('\n', flog);
        }

        close(Csocket);
    }

    close(Ssocket);
    fclose(flog);

    return 0;
}
