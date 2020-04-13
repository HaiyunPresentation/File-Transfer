#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5000

#define BUFFSIZE 64

#define MAXBUFF 128

/* 函数结构声明， command.c文件中定义函数 */

int init(struct sockaddr_in *ser_addr, int *lis_fd, int sock_opt);

int do_put(int sockfd, char *file);

int do_get(int sockfd, char *file);

int do_ls(int sockfd, char *path);

int do_cd(int sockfd, char *path);




// 初始化服务器

// ser_addr: 服务端地址结构指针; lis_fd: 监听套接字描述符; sock_opt: 套接字选项

int init(struct sockaddr_in *ser_addr, int *lis_fd, int sock_opt)

{

    int fd;

    bzero(ser_addr, sizeof(struct sockaddr_in)); // bzero

    ser_addr->sin_family = AF_INET; // AF_INET

    ser_addr->sin_addr.s_addr = htonl(INADDR_ANY); // htonl(INADDR_ANY)

    ser_addr->sin_port = htons(PORT); // htons(PORT)

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // socket 创建监听套接字

    {

        perror("fail to creat socket");

        return -1;
    }

    // 设置套接字选项

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));

    // bind 绑定客户端地址

    if (bind(fd, (struct sockaddr *)ser_addr, sizeof(struct sockaddr_in)) < 0)

    {

        perror("fail to bind");

        return -2;
    }

    //  listen 监听套接字，与客户端的connec函数相互作用

    if (listen(fd, 20) < 0)

    {

        perror("fail to listen");

        return -3;
    }

    *lis_fd = fd;

    return 0;
}

/* 处理来自客户端的GET命令: GET argarg2

* 服务端读取客户端指定的文件，并写入至套接字

* sock_fd: 连接套接字描述符

* file: 客户端请求的文件及其路径

*/

int do_put(int sockfd, char *file)

{

    struct stat stat_buf;

    int len, fd;

    char buf[BUFFSIZE];

    int res = -1;

    if ((fd = open(file, O_RDONLY)) < 0) // open 客户端请求的文件

    {

        write(sockfd, "ERROR: fail to open server file\n",

              strlen("ERROR: fail to open server file\n"));

        return -1;
    }

    if (fstat(fd, &stat_buf) < 0) // struct stat 文件状态

    {

        write(sockfd, "ERROR: fail to stat server file\n",

              strlen("ERROR: fail to stat server file\n"));

        goto end;
    }

    if (!S_ISREG(stat_buf.st_mode)) // 若不是普通文件，则报错

    {

        write(sockfd, "ERROR: not a regular file\n",

              strlen("ERROR: not a regular file\n"));

        goto end;
    }

    sprintf(buf, "OK. FILE SIZE: %d", stat_buf.st_size);

    write(sockfd, buf, strlen(buf)); // 向客户端发送应答信息：OK 文件大小

    read(sockfd, buf, MAXBUFF); // 等待客户端的应答信息，应答码为RDY

    while ((len = read(fd, buf, MAXBUFF)) > 0) // 循环读取文件内容，并写入通信套接字

        write(sockfd, buf, len);

    if (len < 0&& errno == EINTR)

    {

        perror("fail to read");

        goto end;
    }

    printf("OK\n");

    res = 0;

end:

    close(fd); // 关闭文件，注意不是关闭套接字

    return res;
}

/* 处理客户端的PUT请求: put argarg2

* 读取客户端写在通信套接字中的文件内容，并写入至文件

* sockfd: 连接套接字的描述符

* file: 指定的目标文件名及其路径

*/

int do_get(int sockfd, char *file)

{

    struct stat stat_buf;

    char buf[MAXBUFF];

    int fd, len;

    int res = -1;

    fprintf(stdout, "===getting file: %s\n", file);

    // open 打开文件。打开方式是覆盖写，若文件存在则覆盖，但若是一个同名的目录则报错

    if ((fd = open(file, O_RDONLY | O_CREAT | O_TRUNC, 0644)) < 0)

    {

        if (errno == EISDIR) // 不是普通文件，而是一个目录

        {

            write(sockfd, "ERROR: server has a dir with the same name\n",

                  strlen("ERROR: server has a dir with the same name\n"));

            goto end;
        }

        else

        {

            write(sockfd, "ERROR: fail to open server file\n",

                  strlen("ERROR: fail to open server file\n"));

            goto end;
        }
    }

    if (fstat(fd, &stat_buf) < 0) // fstat 获取文件状态

    {

        write(sockfd, "ERROR: fail to stat server file\n",

              strlen("ERROR: fail to stat server file\n"));

        goto end;
    }

    if (!S_ISREG(stat_buf.st_mode)) // 如果不是普通文件，则报错

    {

        write(sockfd, "ERROR: not a regular file\n",

              strlen("ERROR: not a regular file\n"));

        res = 0;

        goto end;
    }

    // 向客户端发送应答码

    write(sockfd, "OK\n", 4);

    while ((len = read(sockfd, buf, MAXBUFF)) > 0)

        write(fd, buf, len);

    if (len < 0&& errno == EINTR)

    {

        perror("fail to read");

        goto end;
    }

    printf("OK\n");

    res = 0;

end:

    close(fd);

    return res;
}

/* 处理LS命令: LS arg1

* sockfd: 已连接的通信套接字描述符

* path: 客户端指定的路径

*/

int do_ls(int sockfd, char *path)

{

    struct stat stat_buf;

    char cmd[BUFFSIZE], buf[MAXBUFF];

    int fd, len;

    int res = -1;

    sprintf(cmd, "ls %s > temp.txt", path); // 拼接命令

    fprintf(stdout, "===from client: system(%s)\n", cmd);

    system(cmd); // system 执行命令

    if ((fd = open("temp.txt", O_RDONLY)) < 0) // open 打开文件

    {

        write(sockfd, "ERROR: fail to ls server file\n",

              strlen("ERROR: fail to ls server file\n"));

        return -1;
    }

    /*      if(fstat(fd, &stat_buf) < 0)    

{

write(sockfd, "ERROR: fail to stat server file\n", 

strlen("ERROR: fail to stat server file\n"));

goto end;

}

if(!S_ISREG(stat_buf.st_mode))

{

write(sockfd, "ERROR: not a regular file\n",

strlen("ERROR: not a regular file\n"));

res = 0;

goto end;

}

fprintf(stdout, "===to client: OK %d\n", stat_buf.st_size);

sprintf(cmd, "OK %d", stat_buf.st_size);

write(sockfd, cmd, strlen(cmd)+1); 

*/

    write(sockfd, "OK\n", 4); // 向客户端发送应答信息

    read(sockfd, cmd, BUFFSIZE); // 等待客户端的应答信息，应答码为RDY

    while ((len = read(fd, buf, MAXBUFF)) > 0) // 循环读写文件内容，并写入至通信套接字

        write(sockfd, buf, len);

    if (len < 0)

    {

        perror("fail to read");

        goto end;
    }

    printf("!ls OK\n");

    res = 0;

end:

    close(fd);

    //      unlink("temp.txt");     // unlink 删除该临时文件

    return res;
}

/* 处理客户端的CD命令: CD arg*/

int do_cd(int sockfd, char *path)

{

    if (chdir(path) < 0) // chdir 改变当前工作目录，进入指定目录

    {

        perror("fail to change directory\n");

        write(sockfd, "ERROR: cannot change server directory\n",

              strlen("ERROR: cannot change server directory\n"));

        return -1;
    }

    write(sockfd, "OK\n", 3);

    return 0;
}