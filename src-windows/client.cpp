#include<iostream>
#include<WinSock2.h>
#include<winsock.h>
#include<Windows.h>
#include<string>
#include<cstring>
#include <fstream>
#include <io.h>
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 1024
#define SERVER_ID "127.0.0.1"
#define FILE_LENGTH 20
#define PORT 5000
using namespace std;
char sendBuff[BUF_SIZE];
char recvBuff[BUF_SIZE];
char fileName[FILE_LENGTH];

int main() {
	WSADATA wsa;
	//ʹ��2.2�汾��soket
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Initialization failed." << endl;
		return -1;
	}
	//����socket
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == -1) {
		cout << "Create Socket Error." << endl;
		return -1;
	}
	//��������ʼ��һ����ַ�ṹ
	sockaddr_in sadr;
	sadr.sin_family = AF_INET;
	sadr.sin_port = htons(PORT);
	sadr.sin_addr.S_un.S_addr = inet_addr(SERVER_ID);
	int nAddrlen = sizeof(sadr);
	while (true) {
		cout << "SENDING..." << endl;
		cout << "Please input the filename: " << endl;
		cin >> fileName;
		FILE* fp;
		if (!(fp = fopen(fileName, "rb"))) {
			cout << "Fail to open file." << endl;
			continue;
		}
		//�����ļ���
		sendto(client, fileName, strlen(fileName), 0, (sockaddr*)&sadr, sizeof(sadr));
		int length;
		int ret;
		while ((length = fread(sendBuff, 1, BUF_SIZE, fp)) > 0) {
			ret = sendto(client, sendBuff, length, 0, (sockaddr*)&sadr, sizeof(sadr));
			if (!ret) {
				cout << "An error occurred while sending." << endl;
				return -1;
			}
			ret = recvfrom(client, recvBuff, BUF_SIZE, 0, (sockaddr*)&sadr, &nAddrlen);
			if (!ret) {
				cout << "Fail to receive." << endl;
				return -1;
			}
			else {
				if (strcmp(recvBuff, "success")) {
					cout << "Fail to receive." << endl;
					return -1;
				}
			}
		}
		//�����ļ����ͽ�����Ϣ
		char end_flag[10] = "end";
		ret = sendto(client, end_flag, length, 0, (sockaddr*)&sadr, sizeof(sadr));
		cout << "successfully sent!" << endl;
		fclose(fp);
	}
	closesocket(client);
	WSACleanup();
	return 0;
}
