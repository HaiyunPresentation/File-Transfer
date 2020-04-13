#include <stdio.h>
#include <stdlib.h> // exit
#include <unistd.h> // STDOUT_FILENO
#include <string.h>
#include <sys/stat.h>   // struct stat
#include <fcntl.h>      // O_WRONLY
#include <sys/socket.h> // struct sockaddr_in
#include <netinet/in.h>
#define PORT 5000
#define BUFFSIZE 64
#define MAXBUFF 128
#define DEBUG_PRINT 1

#ifdef DEBUG_PRINT

#define DEBUG(format, ...) printf("FILE: "__FILE__            \
                                  ", LINE: %d: " format "\n", \
                                  __LINE__, ##__VA_ARGS__)

#else

#define DEBUG(format, ...)

#endif

// 全局变量声明：命令结构，存储用户输入的命令和参数

struct str_command
{

    char *name;

    char *argv[10];
};

/* 函数接口声明 */

// 文件input.c中，处理用户输入

extern int split(struct str_command *command, char *cline);

// 文件command.c中，命令处理

extern int do_connect(char *ip, struct sockaddr_in *serv_addr, int *sock_fd);

extern int do_get(const char *src, const char *dest, int sock_fd);

extern int do_put(const char *src, const char *dest, int sock_fd);

extern int do_cd(char *path);

extern int do_ls(char *path);

extern int do_ser_ls(char *path, int sockfd);

extern int do_ser_cd(char *path, int sockfd);

extern int do_quit(int sock_fd);

int do_connect(char *ip, struct sockaddr_in *serv_addr, int *sock_fd)

{
    //printf("Running: %d\n",__LINE__);
    bzero(serv_addr, sizeof(struct sockaddr_in)); // bzero 清空地址结构

    serv_addr->sin_family = AF_INET; // 使用IPv4地址族

    // inet_pton 将点分十进制的ip地址转换为二进制形式，并存储在地址结构中

    inet_pton(AF_INET, ip, &(serv_addr->sin_addr));

    serv_addr->sin_port = htons(PORT); // htons 将端口号转换为网络字节序存储在地址结构中

    *sock_fd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字

    if (*sock_fd < 0)

    {

        perror("fail to creat socket");

        return -1;
    }

    // 使用该套接字，和填充好的地址结构进行连接

    if (connect(*sock_fd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0)

    {

        perror("fail to connect");

        return -2;
    }

    return 0;
}

/* 处理get命令：get argarg2

* 从服务器端取得文件，文件已存在则覆盖

* src：源文件的绝对路径，dest：目的目录的绝对路径，sock_fd: 通信用的套接字描述符

* client将src_filename传递给server，由server读取文件内容并写入套接字，client读取套接字并写入至文件

*/

int do_get(const char *src, const char *dest, int sock_fd)

{
    //printf("Running: %d\n",__LINE__);
    char *dest_file; // 目的路径，dest+filename
    //printf("Running: %d\n",__LINE__);
    struct stat stat_buf; // struct stat 文件状态
    //printf("Running: %d\n",__LINE__);
    char *p, buf[MAXBUFF];
    //printf("Running: %d\n",__LINE__);
    int fd, len;
    //printf("Running: %d\n",__LINE__);
    int res = -1; // 返回值
    if (src == NULL || dest == NULL) // 检查源文件和目的地址是不是空串

    {

        printf("ERROR: wrong command\n");
        return -1;
    }
    //printf("Running: %d\n",__LINE__);
    // 如果源文件路径的最后一个字符是/，则说明源文件不是普通文件，而是目录
    //printf("Running: %d\n",__LINE__);
    if (src[strlen(src) - 1] == '/')

    {

        printf("source file should be a regular file\n");

        return -2;
    }
    //printf("Running: %d\n",__LINE__);
    // malloc 为目标文件路径分配存储空间，由目标目录dest和源文件名组成
    
    if ((dest_file = (char *)malloc(sizeof(char) * (strlen(dest) + strlen(src)))) == NULL)

    {

        perror("fail to malloc");

        return -3;
    }
    strcpy(dest_file, dest);
    if (dest_file[strlen(dest) - 1] != '/')
        strcat(dest_file, "/");
    p = rindex(src, '/'); // rindex 取源文件路径中最后一个/的位置指针
    //printf("p: %s\n",p);
    //printf("Running: %d\n",__LINE__);
    strcat(dest_file, p + 1);
    //printf("Running: %d\n",__LINE__);
    printf("des: %s\n",dest_file);
    if ((fd = open(dest_file, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0)

    {

        perror("fail to open dest_file");

        goto end2;
    }

    if (fstat(fd, &stat_buf) < 0)

    {

        perror("fail to stat dest_file");

        goto end1;
    }
    //printf("Running: %d\n",__LINE__);
    // S_ISREG

    // 如果目标文件已存在，但不是一个普通文件，则无法传输

    // 否则会造成已存在的目录等其它特殊文件被覆盖

    if (!S_ISREG(stat_buf.st_mode))

    {

        printf("dest-file should be a regular file\n");

        goto end1;
    }

    // 向服务器server发送GET请求

    sprintf(buf, "GET %s", src);

    write(sock_fd, buf, strlen(buf) + 1);

    // 服务器的确认信息格式为：“OK 文件名”

    len = read(sock_fd, buf, MAXBUFF);

    // 如果收到的信息是ERR，表示出错
    //printf("Running: %d\n",__LINE__);
    if (buf[0] == 'E')

    {
        printf("Running: %d\n",__LINE__);
        write(STDOUT_FILENO, buf, len);

        res = 0;

        goto end1;
    }

    // len = atoi(&buf[3]);

    // 告知服务器已准备好RDY，服务器将开始传送文件内容

    write(sock_fd, "RDY", 3);
    //printf("Running: %d\n",__LINE__);
    // 循环读写

    // read 套接字中服务器传入的内容，write 将读取的内容写至目标文件

    while ((len = read(sock_fd, buf, MAXBUFF)) > 0)

    {

        write(fd, buf, len);
    }
    //printf("Running: %d\n",__LINE__);
    if (len < 0)

    {

        printf("ERROR: read\n");

        goto end1;
    }

    printf("OK\n");
    //printf("Running: %d\n",__LINE__);
    res = 0;

end1:

    close(fd);

end2:

    free(dest_file); // free 释放malloc分配的内存空间

    return res;
}

/*

* 处理put命令：put argarg2

* 向服务器传送文件，若已存在则覆盖

* src：源文件的绝对路径，dest: 目标目录的绝对路径, sock_fd 通信用的套接字描述符

* client读取用户指定的src_filename文件内容，并写入至通信套接字；server读取套接字，并写入至文件

*/

int do_put(const char *src, const char *dest, int sock_fd)

{

    char *dest_file; // 目标文件路径，由dest+filename

    struct stat stat_buf; // struct stat 文件状态

    char *p, buf[MAXBUFF];

    int fd, len;

    if (src == NULL || dest == NULL) // 检查源文件和目的地址是不是空串

    {

        printf("ERROR: wrong command\n");

        return -1;
    }

    if (src[strlen(src) - 1] == '/') // 源文件名及其绝对路径

    {

        printf("source file should be a regular file.\n");

        return -1;
    }

    // malloc 为目标文件名及其路径分配内存空间

    if ((dest_file = (char *)malloc(sizeof(char) * (strlen(src) + strlen(dest)))) == NULL)

    {

        perror("fail to malloc");

        return -1;
    }

    strcpy(dest_file, dest);

    if (dest_file[strlen(dest_file) - 1] != '/')

        strcat(dest_file, "/");

    p = rindex(src, '/');

    strcat(dest_file, p + 1);

    // open 打开需要传输的源文件

    if ((fd = open(src, O_RDONLY)) < 0) // open, int fd, write read

    {

        perror("fail to src-file");

        goto end1;
    }

    if (fstat(fd, &stat_buf) < 0) // struct stat 源文件状态

    {

        perror("fail to open src-file");

        goto end2;
    }

    if (!S_ISREG(stat_buf.st_mode)) // 只能是普通文件

    {

        fprintf(stderr, "src-file should be a regular file\n");

        goto end2;
    }

    sprintf(buf, "PUT %s", dest_file); // 向服务器发送PUT请求

    write(sock_fd, buf, strlen(buf) + 1);

    read(sock_fd, buf, BUFFSIZE); // 接收服务器的确认信息

    if (buf[0] == 'E') // 若收到的信息是ERR，表示出错；否则得到RDY应答

    {

        write(STDOUT_FILENO, buf, strlen(buf) + 1);

        goto end2;
    }

    // 循环读取文件内容，并写入至通信套接字传输给服务端

    while ((len = read(fd, buf, MAXBUFF)) > 0)

        write(sock_fd, buf, len);

    if (len < 0) // 读操作出错

    {

        perror("fail to read");

        goto end2;
    }

    printf("OK\n");

end1:

    close(fd);

end2:

    free(dest_file); // free 释放malloc分配的内存空间

    return 0;
}

/*

* 处理ls命令：ls arg1

* path: 指定的目录，绝对路径

*/

int do_ls(char *path)

{
    if(path==NULL)
    {
        path = (char*)malloc(sizeof(char));
        memset(path,'\0',sizeof(path));
    }
    char cmd[128], buf[MAXBUFF];

    FILE *fp;

    sprintf(cmd, "ls %s > temp.txt", path); // 拼接命令

    system(cmd); // system 执行命令

    if ((fp = fopen("temp.txt", "r")) == NULL) // fopen, FILE *fp, fgets fputs

    {

        perror("fail to ls");

        return -1;
    }

    while (fgets(buf, MAXBUFF, fp) != NULL) // fgets, fputs

        fputs(buf, stdout);

    fclose(fp);

    //unlink("temp.txt");

    return 0;
}

/* 处理!ls命令: !ls arg1

* 列出服务器中指定目录的所有文件

* path: 指定的目录，绝对路径

*/

int do_ser_ls(char *path, int sockfd)

{

    char cmd[BUFFSIZE], buf[MAXBUFF];

    int len;

    sprintf(cmd, "LS %s", path);

    if (write(sockfd, cmd, strlen(cmd) + 1) < 0) // write 向服务器发送LS请求

        return -1;

    DEBUG("===to server: %s", cmd);

    if ((len = read(sockfd, cmd, BUFFSIZE)) < 0) // read 读取服务器的应答码

        return -2;

    if (cmd[0] == 'E') // 若应答码为ERR，表示出错

    {

        write(STDOUT_FILENO, cmd, len);

        return 0;
    }

    //len = atoi(&buf[3]);

    DEBUG("===from server: %s", cmd);

    if (write(sockfd, "RDY", 4) < 0) // 告知服务器已准备好RDY

        return -3;

    // read, write 循环读取服务端传输的内容，并输出到屏幕

    while ((len = read(sockfd, buf, MAXBUFF)) > 0)

        write(STDOUT_FILENO, buf, len);

    if (len < 0)

    {

        perror("fail to read");

        return -4;
    }

    printf("!ls OK\n");

    return 0;
}

/* 处理cd命令：cd arg

* path: 指定的目录，绝对路径

*/

int do_cd(char *path)

{

    if (chdir(path) < 0) // chdir 改变当前工作目录

    {

        perror("fail to change directory");

        return -1;
    }

    return 0;
}

/*

* 处理!cd命令: !cd arg1

* 进入服务器中指定的目录

* path: 指定的目录，绝对路径

* sockfd: 通信套接字描述符

*/

int do_ser_cd(char *path, int sockfd)

{

    char buf[BUFFSIZE];

    int len;

    sprintf(buf, "CD %s", path);

    if (write(sockfd, buf, strlen(buf)) < 0) // write 向服务器发送CD请求

        return -1;

    if ((len = read(sockfd, buf, BUFFSIZE)) < 0) // read 读取服务器的应答信息

        return -2;

    if (buf[0] == 'E') // 若应答码为ERR，表示出错

        write(STDOUT_FILENO, buf, len);

    return 0;
}

/* 处理quit命令: quit

* 向服务器发送 关闭连接的请求，然后退出客户端程序

* sockfd: 通信用的套接字描述符

* 这次通信不需要应答码，因为客户端程序发送命令后已退出，无法处理应答码

*/

int do_quit(int sock_fd)

{

    char buf[4];

    sprintf(buf, "BYE");

    // write 向服务器发送关闭连接的请求

    if (write(sock_fd, buf, strlen(buf) + 1) != strlen(buf))

        return -1;

    return 0;
}

void del_blank(int *p,char* cline)
{
    //printf("Running %d\n",__LINE__);
    //printf("P: %d\n",*p);
    while (cline[*p] != '\0' && (cline[*p] == ' ' || cline[*p] == '\t'))
        (*p)++;
}

void get_arg(char *arg,int *p,char* cline)
{

    int j = 0;
    while (cline[*p] != '\0' && cline[*p] != ' ' && cline[*p] != '\t')
        arg[j++] = cline[(*p)++];
}

/* 将用户输入的命令字符串分割为命令和参数，

* 并存储在自定义的 struct str_command中

* command: 存储命令和参数的结构体； cline 用户输入的命令字符串

*/

int split(struct str_command *command, char *cline)

{

    int i = 0, p = 0;
    //printf("Running %d\n",__LINE__);
    cline[strlen(cline) - 1] = '\0'; // 将换行符\n替换为结束符\0

    del_blank(&p, cline); // 过滤空格，直到遇到第一个参数

    while (cline[p] != '\0')

    {
        if ((command->argv[i] = (char *)malloc(sizeof(char) * BUFFSIZE)) == NULL)

        {

            perror("fail to malloc");

            return -1;
        }

        get_arg(command->argv[i], &p, cline);
        /*{

int j = 0;

while(cline[p]!='\0' && cline[p]!=' ' && cline[p]!='\t') 

command->argv[i][j++] = cline[p++]; 



}*/
        //printf("P: %d\n",p);
        i++;
        //printf("Running %d\n",__LINE__);
        del_blank(&p, cline);
    }
    //printf("Running %d\n",__LINE__);
    command->argv[i] = NULL; // 命令参数数组以NULL结尾
    command->name = (char*)malloc(129*sizeof(char));
    command->name = command->argv[0]; // 命令名和第一个参数指向同一内存区域

    return i;
}