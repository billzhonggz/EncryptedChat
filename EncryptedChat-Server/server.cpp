/*
Encrypted Chatroom Server
Assignment 2
Internet & World Wide Web
1430003013 刘毓健 & 1430003045 钟钧儒
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include<string>
#include <time.h>
using namespace std;

#define DEFAULT_PORT	5019
#define DEFAULT_BUFFER 4096

//客户连接池，保存各个客户的连接
vector<SOCKET> clients;

typedef struct
{
	SOCKET sock;
	sockaddr_in addr;
	ofstream* out;
} object;

DWORD WINAPI clientThread(LPVOID lpParam)
{
	object* obj = (object*)lpParam;
	SOCKET sock = obj->sock;
	ofstream* fout = obj->out;

	char buf[DEFAULT_BUFFER] = "";
	char sendback[DEFAULT_BUFFER] = "";
	int ret, left, idx = 0;

	//循环处理客户端发送的信息
	while (1)
	{
		ret = recv(sock, buf, DEFAULT_BUFFER, 0);
		if (ret == 0)
			return 1;
		if (ret == SOCKET_ERROR)
		{
			printf("receive message failed:%d\n", WSAGetLastError());
			return 1;
		}

		buf[ret] = '\0';

		sprintf(sendback, "[%s said]:%s\n", inet_ntoa(obj->addr.sin_addr), buf);

		printf("[The message from %s]:%s\n", inet_ntoa(obj->addr.sin_addr), buf);

		printf("[The sendback message is]:  %s\n", sendback);

		//向各个客户端发送通告信息
		for (int i = 0; i < clients.size(); i++)
		{

			left = strlen(sendback);
			while (left > 0)
			{
				ret = send(clients[i], &sendback[idx], left, 0);

				if (ret == 0)
					return 1;
				else if (ret == SOCKET_ERROR)
				{
					printf("send back message failed:%d\n", WSAGetLastError());
					return 1;
				}

				left -= ret;
				idx += ret;

			}
			idx = 0;
		}

		memset(buf, 0, DEFAULT_BUFFER);
		memset(sendback, 0, DEFAULT_BUFFER);
	}

	return 0;
}



int main(void) {

	//char szBuff[100];
	//int msg_len;
	int addr_len;
	struct sockaddr_in local, client_addr;
	string record;

	SOCKET listensock, client;
	WSADATA wsaData;
	HANDLE hThread;
	DWORD  threadId;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR) {
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}
	// Fill in the address structure
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(DEFAULT_PORT);

	listensock = socket(AF_INET, SOCK_STREAM, 0);	//TCp socket


	if (listensock == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if (bind(listensock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR) {
		fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//waiting for the connections
	if (listen(listensock, 5) == SOCKET_ERROR) {
		fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Waiting for the connections ........\n");

	while (1)
	{
		addr_len = sizeof(client_addr);
		client = accept(listensock, (struct sockaddr*)&client_addr, &addr_len);
		clients.push_back(client);
		printf("Online client number: %d\n", clients.size());

		if (client == INVALID_SOCKET)
		{
			fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
			break;
		}

		object obj;
		obj.addr = client_addr;
		obj.sock = client;
		//为每个客户建立处理线程
		hThread = CreateThread(NULL, 0, clientThread, (LPVOID)&obj, 0, &threadId);

		if (hThread == NULL)
		{
			fprintf(stderr, "create thread failed with error %d\n", WSAGetLastError());
				break;
		}
	}

	closesocket(listensock);
	WSACleanup();

	return 0;
}