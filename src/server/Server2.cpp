#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <WinSock2.h>  
#include<iostream>
#include "MD5.h"
#include <fstream>
#include<cstring>
#include <process.h>
using namespace std;
#define PORT 2400  
#define SERVER_IP "127.0.0.1"  
#define BUFFER_SIZE 1024  
#define fileNAME_MAX_SIZE 512
#pragma comment(lib, "WS2_32")  
char fileName[fileNAME_MAX_SIZE];//文件路径
char buffer[BUFFER_SIZE];
char md5_str[100];
int main()
{

	// 初始化socket dll  
	WSADATA wsaData;
	WORD socketVersion = MAKEWORD(2, 0);
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		printf("Init socket  error!");
		exit(1);
	}

	// 指定服务端(本地)的地址 
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	// 创建服务器端socket  
	SOCKET Serv_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == Serv_Socket)
	{
		printf("Create Socket Error!");
		exit(1);
	}
	int opt = 1;
	setsockopt(Serv_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
	
	//绑定
	if (SOCKET_ERROR == bind(Serv_Socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Failed: %d", WSAGetLastError());
		exit(1);
	}

	//监听  
	if (SOCKET_ERROR == listen(Serv_Socket, 10))
	{
		printf("Server Listen Failed: %d", WSAGetLastError());
		exit(1);
	}

	sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	SOCKET Clnt_Socket;
	Clnt_Socket = accept(Serv_Socket, (struct sockaddr *)&client_addr, &client_addr_len);

	if (SOCKET_ERROR == Clnt_Socket)
	{
		printf("Server Accept Failed: %d", WSAGetLastError());

	}

	while (1)
	{
		printf("Listening To Client...\n");
		memset(buffer, 0, BUFFER_SIZE);
		if (recv(Clnt_Socket, buffer, BUFFER_SIZE, 0) < 0)
		{
			printf("Server Receive Data Failed!");
			break;
		}


		strncpy(fileName, buffer, strlen(buffer) > fileNAME_MAX_SIZE ? fileNAME_MAX_SIZE : strlen(buffer));
		fileName[strlen(buffer)] = '\0';
		int flag = 0;//不存在md5文件
		cout << "will transfer: " << fileName << endl;

		if (!(fopen("MD5.txt", "rb")))
		{
			cout << "start to send"<<endl;
		}
		else
		{
			FILE * fp_md5_read = fopen("MD5.txt", "rb");
			fgets( md5_str,100, fp_md5_read);
			flag = 1;//MD5文件存在

		}
		//判断发送过的文件是否完整
		if (flag == 1)
		{
			//计算发送文件MD5
			ifstream o(fileName, ios::in | ios::binary);
			MD5 fileMd5_1(o);
			
			if (strcmp(md5_str, fileMd5_1.toString().c_str())==0)
			{
				cout << "文件已完整传输过"<<endl;
				flag = 1;
				
			}
			else
			{
				
				cout << "文件未传输完整" << endl;
			}
		}
		//未发送过该文件则发送
		if (flag == 0)
		{
			FILE * fp = fopen(fileName, "rb"); //windows下是"rb",表示打开一个只读的二进制文件  
			if (NULL == fp)
			{
				cout << "can not open source file" << endl;
				system("pause");
			}
			else
			{

				memset(buffer, 0, BUFFER_SIZE);
				int length = 0;
				int file_length = 0;
				while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
				{
					if (send(Clnt_Socket, buffer, length, 0) < 0)
					{

						cout << "send file failed" << endl;
						break;
					}
					file_length = length + file_length;
					memset(buffer, 0, BUFFER_SIZE);
				}

				fclose(fp);

				cout << "Send " << fileName << "Successful!" << endl;
				cout << "已传文件大小为:" << file_length << endl;

				//计算发送文件MD5
				ifstream of(fileName, ios::in | ios::binary);
				MD5 fileMd5(of);
				FILE * fp_md5;
				if (fp_md5 = fopen("MD5.txt", "wb"))
				{

					
					fprintf(fp_md5, fileMd5.toString().c_str());


					fclose(fp_md5);
					cout << "create MD5.txt Successful!" << endl;
				}
				else cout << "写入MD5失败" << endl;

			}
		}
		closesocket(Clnt_Socket);
	}
	
	closesocket(Serv_Socket);
	//释放winsock库  
	WSACleanup();
	system("pause");
	return 0;
}