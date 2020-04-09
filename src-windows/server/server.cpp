
#include<iostream>
#include<WinSock2.h>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#define LENGTH_OF_LISTEN_QUEUE 10 
#define BUFFER_SIZE 1024
#define FILE_LENGHT 20
#define PORT 5000
using namespace std;

char sendBuff[BUFFER_SIZE];
char recvBuff[BUFFER_SIZE];
char fileName[FILE_LENGHT];

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Initialization failed." << endl;
		return -1;
	}
	SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET) {
		cout << "Socket failed." << endl;
		return -1;
	}
	sockaddr_in my_addr, remote_addr;
	int nAddrlen = sizeof(remote_addr);
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(server_socket, (sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR) {
		cout << "Bind error!" << endl;
		return -1;
	}

	if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE) == SOCKET_ERROR)
	{
		cout << "listen error !" << endl;
		return -1;
	}
	SOCKET sClient;
	while (true) {
		cout << "RECEIVING..." << endl;
		//�����ļ���
		sClient = accept(server_socket, (SOCKADDR*)&remote_addr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			cout << "accept error !" << endl;
			continue;
		}
		else
			cout << "Successfully accept" << endl;
		int ret = recv(sClient, fileName, FILE_LENGHT, 0);
		cout << "Filename: " << fileName << endl;
		//errno_t err;
		FILE* fp;
		if (!(fp = fopen(fileName, "wb"))) {
			cout << "Create failed." << endl;
			return -1;
		}
		char sendInfo[5] = "OK";
		send(sClient, sendInfo, 5, 0);
		int length;
		while (length = recv(sClient, recvBuff, BUFFER_SIZE, 0) > 0) {
			//���ս�����Ϣ
			if (!strcmp(recvBuff, "end")){
				cout << "Successfully received" << endl;
				break;
			}
			if (length == 0) {
				cout << "An error occurred while receiving." << endl;
				return -1;
			}
			int ret = fwrite(recvBuff, 1, length, fp);
			if (ret < length) {
				cout << "Write failed." << endl;
				return -1;
			}
			send(sClient, sendInfo, 5, 0);
		}
		fclose(fp);
	}
	closesocket(server_socket);
	WSACleanup();
	return 0;
}
