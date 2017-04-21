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
#include "rsa.h"

#define  DEFAULT_PORT	5019
void divideUsernameMessage(char *input, char *username, char *message);

typedef struct
{
	SOCKET clientSock;
	char *username;
	int prime1;
	int prime2;
	long int publicKey;  // TEMP: Should fit with all other public keys.
	long int privateKey; // TEMP: To test encrypted message. Should move to recieve thread. 
}sendThreadPara;

typedef struct
{
	SOCKET clientSock;
	int prime1;
	int prime2;
	long int privateKey;
}recieveThreadPara;

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
	char dest[DEFAULT_BUFFER] = "";
	char input[DEFAULT_BUFFER] = "";
	int bytesSent, left, idx = 0;

	//采取循环形式以确认信息完整发出，这是因为内核输出缓存有限制，输入信息有可能超过缓存大小
	while (1)
	{
		printf("Input your destination user. Input \"server\" to execute server commands.\n");
		// Push username of the sender at the front of the sending buffer.
		strcat(sendbuf, "[");
		strcat(sendbuf, username);
		strcat(sendbuf, "]");
		//fgets(input, DEFAULT_BUFFER, stdin);
		scanf("%s", &dest);
		getchar();
		printf("Destination is %s\n", dest);
		// Combine destination to the send buff.
		strcat(sendbuf, "{");
		strcat(sendbuf, dest);
		strcat(sendbuf, "}");
		
		// Input message
		printf("Input your message or server command.\n");
		scanf("%s", &input);
		getchar();
		printf("Message is %s\n", input);

		if (strcmp(dest, "server") != 0)
		{
			// Do encryption.
			// TODO: Free memory when finish. 
			char *encryptedInput = doEncrypt(input, prime1, prime2, publicKey);
			printf("Encrypted input is %s\n", encryptedInput);
			// Combine sender's username at the front of the send information.
			strcat(sendbuf, encryptedInput);
			free(encryptedInput);
		}
		else
		{
			// Direct combine and send. 
			strcat(sendbuf, input);
		}
		left = strlen(sendbuf);
		printf("Ready to send: %s\nLength: %d.\n", sendbuf, left);

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
	recieveThreadPara* recievePara = (recieveThreadPara*)lpParam;
	SOCKET sock = recievePara->clientSock;
	int prime1 = recievePara->prime1;
	int prime2 = recievePara->prime2;
	long int privateKey = recievePara->privateKey;

	int bytesRecv = SOCKET_ERROR;
	char recvbuf[DEFAULT_BUFFER] = "";
	// Receive message.
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
			printf("Recieved: %s", recvbuf);
			// Discard brackets.
			char *sourceUsername = (char*)malloc(DEFAULT_BUFFER * sizeof(char));
			char *encryptedMsg = (char*)malloc(DEFAULT_BUFFER * sizeof(char));
			divideUsernameMessage(recvbuf, sourceUsername, encryptedMsg);
			printf("Source username %s, original message: %s\n", sourceUsername, encryptedMsg);

			// Do decryption.
			char *decryptedMsg = doDecrypt(encryptedMsg, prime1, prime2, privateKey);
			printf("Decrypted message: %s\n", decryptedMsg);
			free(sourceUsername);
			free(encryptedMsg);
			free(decryptedMsg);
		}
		bytesRecv = SOCKET_ERROR;
		memset(recvbuf, 0, DEFAULT_BUFFER);
	}
}

void divideUsernameMessage(char *input, char *username, char *message)
{
	int index = 0;
	int len = strlen(input);
	int start = 0, end = 0;
	for (index; index < len; index++) { //Count how many character in the array
		if (input[index] == '[') {
			start = index;
		}
		if (input[index] == ']') {
			end = index;
			break;
		}
	}
	strncpy(username, input + start + 1, end - start - 1);
	username[strlen(username)] = '\0';
	strncpy(message, input + end + 1, len);
	//strcat(message, '\0');
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
	
	// Create thread for sending. 
	// Assign parameters.
	sendThreadPara sendPara;
	sendPara.clientSock = connect_sock;
	sendPara.username = username;
	sendPara.prime1 = prime1;
	sendPara.prime2 = prime2;
	sendPara.publicKey = publicKey; //TEMP: Should be replaced with the list.
	sendPara.privateKey = privateKey; //TEMP: Should be deleted. 
	hThreadSend = CreateThread(NULL, 0, SendThread, &sendPara, 0, &sendThreadId);

	// Create thread for recieving. 
	// Assign parameters. 
	recieveThreadPara recievePara;
	recievePara.clientSock = connect_sock;
	recievePara.prime1 = prime1;
	recievePara.prime2 = prime2;
	sendPara.privateKey = privateKey;
	hThreadReceive = CreateThread(NULL, 0, ReceiveThread, &recievePara, 0, &receiveThreadId);

	WaitForSingleObject(hThreadSend, INFINITE);
	WaitForSingleObject(hThreadReceive, INFINITE);

	closesocket(connect_sock);
	WSACleanup();
	system("pause");
	return 0;
}