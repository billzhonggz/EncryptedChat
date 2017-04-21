/*
	Encrypted Chatroom Client
	Assignment 2
	Internet & World Wide Web
	1430003013 刘毓健 & 1430003045 钟钧儒
*/

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include"rsa.h"

#define  DEFAULT_PORT	5019

typedef struct
{
	SOCKET clientSock;
	char *username;
	int prime1;
	int prime2;
	long int publicKey;
	long int privateKey;
}sendThreadPara;

DWORD WINAPI SendThread(LPVOID lpParam)
{

	sendThreadPara* sendPara = (sendThreadPara*)lpParam;
	SOCKET sock = sendPara->clientSock;
	char *username = sendPara->username;
	int prime1 = sendPara->prime1;
	int prime2 = sendPara->prime2;
	long int publicKey = sendPara->publicKey;
	long int privateKey = sendPara->privateKey;

	char sendbuf[DEFAULT_BUFFER] = "";
	char input[DEFAULT_BUFFER] = "";
	int bytesSent, left, idx = 0;

	// Identify input format to the user.
	printf("Input your message with the format \"[target]message\"\nUse \"[server]command\" to access server.\nUse \"[server]userlist\" to see online users.\n");

	//采取循环形式以确认信息完整发出，这是因为内核输出缓存有限制，输入信息有可能超过缓存大小
	while (1)
	{
		// Push username of the sender at the front of the sending buffer.
		strcat(sendbuf, "[");
		strcat(sendbuf, username);
		strcat(sendbuf, "]");
		//fgets(input, DEFAULT_BUFFER, stdin);
		scanf("%s", &input);
		printf("Your input is %s\n", input);
		// Do encryption.
		// TODO: Free memory when finish. 
		char *encryptedInput = doEncrypt(input, prime1, prime2, publicKey);
		printf("Encrypted input is %s\n", encryptedInput);
		// TEST: do decryption.
		char *decryptedInput = doDecrypt(encryptedInput, prime1, prime2, privateKey);
		printf("Decrypted input is %s\n", decryptedInput);
		// Combine sender's username at the front of the send information.
		strcat(sendbuf, input);
		left = strlen(sendbuf);
		printf("Ready to send: %sLength: %d.\n", sendbuf, left);

		// Send message.
		while (left > 0)
		{
			bytesSent = send(sock, &sendbuf[idx], left, 0);

			if (bytesSent == 0)
				return 1;
			if (bytesSent == SOCKET_ERROR)
			{
				printf("send failed:%d", WSAGetLastError());
				return 1;
			}
			left -= bytesSent;
			idx += bytesSent;

		}
		idx = 0;
		memset(sendbuf, 0, DEFAULT_BUFFER);
	}
	return 0;
}

DWORD WINAPI ReceiveThread(LPVOID lpParam)
{
	SOCKET sock = (SOCKET)lpParam;
	int bytesRecv = SOCKET_ERROR;
	char recvbuf[DEFAULT_BUFFER] = "";
	while (1)
	{
		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(sock, recvbuf, DEFAULT_BUFFER, 0);

			if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
				printf("Connection Closed.\n");
				return 1;
			}
			if (bytesRecv < 0)
				return 1;
			printf("%s", recvbuf);
		}
		bytesRecv = SOCKET_ERROR;
		memset(recvbuf, 0, DEFAULT_BUFFER);
	}
}

int main(void)
{
	// Define username by user.
	printf("Please input your username, no more than 16 characters.\n");
	char username[17] = { '\0' };
	gets_s(username, 16);
	printf("Your username is %s.\nBegin connection.\n", username);
	int prime1 = 0, prime2 = 0;
	// Ask for two prime numbers.
	while (1)
	{
		printf("Input two prime numbers, spilt with a comma.\n");
		scanf("%d,%d", &prime1, &prime2);
		if (prime(prime1) == 1 && prime(prime2) == 1)
		{
			printf("Your input is %d and %d.\n", prime1, prime2);
			break;
		}
		else
		{
			printf("Your input is incorrect. Please try again.\n");
			continue;
		}
	}

	// Generate a key pair.
	long int* keyPair = rsaGenKeyPair(prime1, prime2);
	long int publicKey = keyPair[0];
	long int privateKey = keyPair[1];
	printf("Generated public key is %d, private key is %d.\n", publicKey, privateKey);

	// Initialize variables for threads.
	HANDLE hThreadSend;
	DWORD  sendThreadId;
	HANDLE hThreadReceive;
	DWORD  receiveThreadId;

	// Initialize variables for winsock.
	struct sockaddr_in server_addr;
	struct hostent *hp;
	SOCKET connect_sock;
	WSADATA wsaData;
	char			*server_name = "localhost";
	unsigned short	port = DEFAULT_PORT;
	unsigned int	addr;

	// Try connect.
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR) {
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}
	if (isalpha(server_name[0]))
		hp = gethostbyname(server_name);
	else {
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char*)&addr, 4, AF_INET);
	}
	if (hp == NULL)
	{
		fprintf(stderr, "Cannot resolve address: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	//copy the resolved information into the sockaddr_in structure
	memset(&server_addr, 0, sizeof(server_addr));
	memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
	server_addr.sin_family = hp->h_addrtype;
	server_addr.sin_port = htons(port);


	connect_sock = socket(AF_INET, SOCK_STREAM, 0);	//TCp socket

	if (connect_sock == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Client connecting to: %s\n", hp->h_name);

	if (connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))
		== SOCKET_ERROR) {
		fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	
	// Create two threads for send and receive. 
	// Assign parameters.
	sendThreadPara sendPara;
	sendPara.clientSock = connect_sock;
	sendPara.username = username;
	sendPara.prime1 = prime1;
	sendPara.prime2 = prime2;
	sendPara.publicKey = publicKey;
	sendPara.privateKey = privateKey;

	hThreadSend = CreateThread(NULL, 0, SendThread, &sendPara, 0, &sendThreadId);
	hThreadReceive = CreateThread(NULL, 0, ReceiveThread, (LPVOID)connect_sock, 0, &receiveThreadId);

	WaitForSingleObject(hThreadSend, INFINITE);
	WaitForSingleObject(hThreadReceive, INFINITE);

	closesocket(connect_sock);
	WSACleanup();
	system("pause");
	return 0;
}