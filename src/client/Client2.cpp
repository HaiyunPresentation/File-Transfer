#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <WinSock2.h>  
#include<iostream>
#include "MD5.h"
#include <fstream>
#include <process.h>
#include <cstdlib>

using namespace std;

#define PORT 2400 
#define SERVER_IP "127.0.0.1"  
#define BUFFER_SIZE 1024  
#define fileNAME_MAX_SIZE 512
#pragma comment(lib, "WS2_32")  
char fileName[fileNAME_MAX_SIZE];
char buffer[BUFFER_SIZE];

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

	//创建客户端socket  
	SOCKET Clnt_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == Clnt_Socket)
	{
		cout << "Create Socket Error!" << endl;
		system("pause");
		exit(1);
	}

	//指定服务端的地址  
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(PORT);
	int opt = 1;
	setsockopt(Clnt_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
	if (SOCKET_ERROR == connect(Clnt_Socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
	{
		printf("Connect failed");
		system("pause");
		exit(1);
	}
	while (1)
	{
		int flag = 0;//文件未传过
		FILE *fp;
		//输入源文件路径，此消息发至服务端
		cout << "please input the file name:" << endl;
		cin >> fileName;

		memset(buffer, 0, BUFFER_SIZE);
		strncpy(buffer, fileName, strlen(fileName) > BUFFER_SIZE ? BUFFER_SIZE : strlen(fileName));

		//向服务器发送文件名  
		if (send(Clnt_Socket, fileName, BUFFER_SIZE, 0) < 0)
		{
			cout << "Send File Name Failed" << endl;
			system("pause");
			exit(1);
		}
		if (!(fp = fopen(fileName, "rb")))
		{
			if (!(fp = fopen(fileName, "wb")))
			{
				cout << "can not open the  file to write" << endl;
			}
			else
			{
				memset(buffer, 0, BUFFER_SIZE);
				int length = 0;
				while ((length = recv(Clnt_Socket, buffer, BUFFER_SIZE, 0)) > 0)
				{
					if (fwrite(buffer, sizeof(char), length, fp) < length)
					{
						//printf("File: %s Write Failed\n", file_name);
						cout << "write failed" << endl;
						break;
					}
					memset(buffer, 0, BUFFER_SIZE);
				}
				cout << "Receive " << fileName << "  Successful!" << endl;
				fclose(fp);

				ifstream of(fileName, ios::in | ios::binary);

				if (!of.is_open())
				{
					cout << "无法打开文件来进行解析MD5";
					return 0;
				}
				MD5 fileMd5(of);
				FILE * fp_md5;
				if (fp_md5 = fopen("MD5.txt", "wb"))
				{

					fprintf(fp_md5, fileMd5.toString().c_str());
					fclose(fp_md5);
					cout << "create MD5.txt Successful!"<<endl;
				}
				else cout << "写入MD5失败" << endl;
			}

			closesocket(Clnt_Socket);
		}
		else
			cout << "文件已存在" << endl;
			closesocket(Clnt_Socket);
	}
	//释放winsock库  
	WSACleanup();

	system("pause");
	return 0;
}