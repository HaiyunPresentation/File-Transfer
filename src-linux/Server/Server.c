#include "common.h"


int main(void)

{

    struct sockaddr_in ser_addr, cli_addr; // 服务/客户端地址结构

    char buf[BUFFSIZE];

    int listenfd, connfd;

    int sock_opt, len;

    pid_t pid;

    // 自定义init初始化，得到地址结构和监听套接字描述符

    if (init(&ser_addr, &listenfd, sock_opt) < 0)

        exit(-1);

    printf("waiting connections ...\n");

    while (1) // while死循环，处理客户端请求

    {

        // accept 接收请求

        if ((connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &len)) < 0)

        {

            perror("fail to accept");

            exit(-2);
        }

        if ((pid = fork()) < 0) // fork 创建子进程

        {

            perror("fail to fork");

            exit(-3);
        }

        if (pid == 0) // 子进程处理连接请求，父进程继续监听

        {

            close(listenfd); // 子进程中关闭继承而来的监听套接字

            // 本程序的客户端是一个交互式程序，服务器端也是交互的

            while (1)

            {

                if (read(connfd, buf, BUFFSIZE) < 0)

                    exit(-4);

                // strstr(str1, str2) 判断str2是否为str1的字串。

                // 若是，则返回str2在str1中首次出现的地址；否则，返回NULL

                if (strstr(buf, "GET") == buf)

                {

                    if (do_put(connfd, &buf[4]) < 0)

                        printf("error occours while putting\n");
                }

                else if (strstr(buf, "PUT") == buf)

                {

                    if (do_get(connfd, &buf[4]) < 0)

                        printf("error occours while getting\n");
                }

                else if (strstr(buf, "CD") == buf)

                {

                    if (do_cd(connfd, &buf[4]) < 0)

                        printf("error occours while changing directory\n");
                }

                else if (strstr(buf, "LS") == buf)

                {

                    if (do_ls(connfd, &buf[3]) < 0)

                        printf("error occours while listing\n");
                }

                else if (strstr(buf, "BYE") == buf)

                    break;

                else

                {

                    printf("wrong command\n");

                    exit(-5);
                }
            }

            close(connfd); // 跳出循环后关闭连接套接字描述符，通信结束

            exit(0); // 子进程退出
        }

        else

            close(connfd); // 父进程关闭连接套接字，继续监听
    }

    return 0;
}