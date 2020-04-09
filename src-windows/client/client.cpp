#include <iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <string>
#include <cstring>
#include <fstream>
#include <io.h>
#pragma comment(lib,"ws2_32.lib")
#define BUFFER_SIZE 1024
#define SERVER_ID "127.0.0.1"
#define FILE_LENGTH 20
#define PORT 5000


using namespace std;
char sendBuff[BUFFER_SIZE];
char recvBuff[BUFFER_SIZE];
char fileName[FILE_LENGTH];

int main() {
	WSADATA wsa;
	//使用2.2版本的socket
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Initialization failed." << endl;
		return -1;
	}
	//构造socket
	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket == INVALID_SOCKET) {
		cout << "Create Socket Error." << endl;
		return -1;
	}
	//声明并初始化一个地址结构
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_ID);

	while (true) {
		cout << "SENDING..." << endl;
		cout << "Please input the filename: " << endl;
		cin >> fileName;
		FILE* fp;
		if (!(fp = fopen(fileName, "rb"))) {
			cout << "Fail to open file." << endl;
			continue;
		}
		if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		{
			cout << "Connect error !" << endl;
			return -1;
		}
		if (send(client_socket, fileName, strlen(fileName)+1, 0) < 0) {
			cout << "fileName send failed!" << endl;
			continue;
		}

		int ret = recv(client_socket, recvBuff, BUFSIZ, 0);
		int length;
		while ((length = fread(sendBuff, 1, BUFFER_SIZE, fp)) > 0) {

			if (send(client_socket, sendBuff, length, 0) < 0) {
				cout << "Send failed!" << endl;
				return -1;
			}
			ret = recv(client_socket, recvBuff, BUFSIZ, 0);
		}
		//传送文件发送结束信息
		char end_flag[10] = "end";
		ret = send(client_socket, end_flag, 10, 0);
		cout << "Successfully sent!" << endl;
		fclose(fp);
	}
	closesocket(client_socket);
	WSACleanup();
	return 0;
}
