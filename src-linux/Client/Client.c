#include "common.h"

int main(void)
{
    //printf("Running: %d\n",__LINE__);
    char cline[MAXBUFF]; // 缓冲区，存储用户输入的命令

    struct str_command command; // 命令结构，存储分解后的命令

    int sock_fd;

    struct sockaddr_in serv_addr; // 服务器端的地址结构

    printf("myftp$: "); // 打印提示符

    fflush(stdout); // fflush 冲洗，保证提示符显示

    while (fgets(cline, MAXBUFF, stdin) != NULL) // fgets 得到一行命令

    {
        //printf("Running: %d\n",__LINE__);
        // 自定义split 将命令行拆分为命令和参数
        int len,i;
        if ((len=split(&command, cline)) < 0)
            exit(-1);
        //printf("Running: %d\n",__LINE__);
        // strcasecmp 忽略大小写进行比较
        /*for(i=0;i<len;i++)
            printf("%d : %s\n",i,command.argv[i]);*/
        if (strcasecmp(command.name, "get") == 0)

        {

            if (do_get(command.argv[1], command.argv[2], sock_fd) < 0)

                exit(-2);
        }

        else if (strcasecmp(command.name, "put") == 0)

        {

            if (do_put(command.argv[1], command.argv[2], sock_fd) < 0)

                exit(-3);
        }

        else if (strcasecmp(command.name, "cd") == 0)

        {

            if (do_cd(command.argv[1]) < 0)

                exit(-4);
        }

        else if (strcasecmp(command.name, "ls") == 0)

        {

            if (do_ls(command.argv[1]) < 0)

                exit(-5);
        }

        else if (strcasecmp(command.name, "connect") == 0)

        {
            //printf("Running: %d\n",__LINE__);
            if (do_connect(command.argv[1], &serv_addr, &sock_fd) < 0)

                exit(-6);
        }

        else if (strcasecmp(command.name, "!ls") == 0)

        {

            if (do_ser_ls(command.argv[1], sock_fd))

                exit(-9);
        }

        else if (strcasecmp(command.name, "!cd") == 0)

        {

            if (do_ser_cd(command.argv[1], sock_fd))

                exit(-10);
        }

        else if (strcasecmp(command.name, "quit") == 0)

        {

            if (do_quit(sock_fd) < 0)

                exit(-8);
        }

        else

        {

            printf("ERROR: wrong command\n");

            printf("Usage: command argvargv2, ...\n");
        }

        bzero(&command, sizeof(struct str_command));

        printf("myftp$: "); // 再次打印提示符，准备接受新的命令

        fflush(stdout);
    }

    if (close(sock_fd) < 0)

    {

        perror("fail to close");

        exit(-7);
    }

    return 0;
}