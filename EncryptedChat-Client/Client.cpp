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

int firstStratFlag = 1;
char *extract_Sender(char *input, char *output);
char *extract_Receiver(char *input, char *output);
char *extract_Message(char *input, char *output);
//void divideUsernameMessage(char *input, char *username, char *message);

typedef struct
{
	SOCKET clientSock;
	char *myUsername;
	int prime1;
	int prime2;
	long int myPublicKey;  // TEMP: Should fit with all other public keys.
}sendThreadPara;

typedef struct
{
	SOCKET clientSock;
	int prime1;
	int prime2;
	long int privateKey;
}receiveThreadPara;

typedef struct
{
	char *username;
	long int publicKey;
	int prime1;
	int prime2;
	struct userNode *next;
}userNode;

// Initialize user list.
void initUserList(userNode **pNode)
{
	*pNode = NULL;
	printf("User list initialized.\n");
}

// Create a userlist.
userNode *addUserToList(userNode *pHead, char *username, long int publicKey, int prime1, int prime2)
{
	userNode *p1;
	userNode *p2;

	// Allocate new node.
	p1 = p2 = (userNode*)malloc(sizeof(userNode));
	if (p1 == NULL || p2 == NULL)
	{
		printf("Add user to list failed: memory allocation failed.\n");
		exit(0);
	}
	memset(p1, 0, sizeof(userNode));

	// Add elements to the new node.
	p1->username = username;
	p1->publicKey = publicKey;
	p1->prime1 = prime1;
	p1->prime2 = prime2;
}

DWORD WINAPI SendThread(LPVOID lpParam)
{
	sendThreadPara* sendPara = (sendThreadPara*)lpParam;
	SOCKET sock = sendPara->clientSock;
	char *username = sendPara->myUsername;
	int prime1 = sendPara->prime1;
	int prime2 = sendPara->prime2;
	long int publicKey = sendPara->myPublicKey;

	char sendbuf[DEFAULT_BUFFER] = "";
	char dest[DEFAULT_BUFFER] = "";
	char input[DEFAULT_BUFFER] = "";
	char publicKeyStr[sizeof(long)] = "";
	char prime1Str[sizeof(int)] = "";
	char prime2Str[sizeof(int)] = "";
	int bytesSent, left, idx = 0;

	//采取循环形式以确认信息完整发出，这是因为内核输出缓存有限制，输入信息有可能超过缓存大小
	while (1)
	{
		// Send hello message.
		if (firstStratFlag == 1)
		{
			strcpy(dest, "server");
			itoa(publicKey,publicKeyStr, 10);
			itoa(prime1, prime1Str, 10);
			itoa(prime2, prime2Str, 10);
			sprintf(input, "(publickey)%s,%s,%s", publicKeyStr, prime1Str, prime2Str);
			firstStratFlag = 0;
		}
		else
		{
			// Ask for destination.
			printf("Input your destination user. Input \"server\" to execute server commands.\n");
			scanf("%s", &dest);
			getchar();
			printf("Destination is %s\n", dest);
			// Ask for input.
			printf("Input your message or server command.\n");
			scanf("%s", &input);
			getchar();
			printf("Message is %s\n", input);
		}
		
		// Push username of the sender at the front of the sending buffer.
		strcat(sendbuf, "[");
		strcat(sendbuf, username);
		strcat(sendbuf, "]");
		//fgets(input, DEFAULT_BUFFER, stdin);
		
		// Combine destination to the send buff.
		strcat(sendbuf, "{");
		strcat(sendbuf, dest);
		strcat(sendbuf, "}");

		if (strcmp(dest, "server") != 0 && strcmp(dest, "publickey") != 0)
		{
			// Do encryption.
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
	receiveThreadPara* recievePara = (receiveThreadPara*)lpParam;
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
			//char *sourceUsername = (char*)malloc(DEFAULT_BUFFER * sizeof(char));
			char sourceUsername[DEFAULT_BUFFER] = "";
			char destUsername[DEFAULT_BUFFER] = "";
			//char *encryptedMsg = (char*)malloc(DEFAULT_BUFFER * sizeof(char));
			char encryptedMsg[DEFAULT_BUFFER] = "";
			//divideUsernameMessage(recvbuf, sourceUsername, encryptedMsg);
			extract_Sender(recvbuf, sourceUsername);
			extract_Receiver(recvbuf, destUsername);
			extract_Message(recvbuf, encryptedMsg);
			printf("\nSource username %s, destnation %s, original message: %s\n", sourceUsername, destUsername, encryptedMsg);

			// TODO: Unique return handling. Public keys list. 
			if (sourceUsername != "server")
			{
				// Do decryption.
				char *decryptedMsg = doDecrypt(encryptedMsg, prime1, prime2, privateKey);
				printf("Decrypted message: %s\n", decryptedMsg);
				free(decryptedMsg);
			}
			else
				printf(encryptedMsg);
		}
		// Reset receive containers. 
		bytesRecv = SOCKET_ERROR;
		memset(recvbuf, 0, DEFAULT_BUFFER);
	}
}

char *extract_Sender(char *input, char *output)
{
	int index = 0;
	int start = 0, end = 0;
	for (index; index < strlen(input); index++) { //Count how many character in the array
		if (input[index] == '[') {
			start = index;
		}
		if (input[index] == ']') {
			end = index;
			break;
		}
	}
	strncpy(output, input + start + 1, end - start - 1);
	//int len = strlen(output);
	//output[len] = '\0';
	return output;
}

char *extract_Receiver(char *input, char *output)
{
	int index = 0;
	int start = 0, end = 0;
	for (index; index < strlen(input); index++) { //Count how many character in the array
		if (input[index] == '{') {
			start = index;
		}
		if (input[index] == '}') {
			end = index;
			break;
		}
	}
	if (end - start > 0) {
		strncpy(output, input + start + 1, end - start - 1);
		return output;
	}
	else
		return "\0";
}

char *extract_Message(char *input, char *output)
{
	int index = 0;
	int start = 0, end = strlen(input);
	for (index; index < end; index++) 
	{
		if (input[index] == '}') {
			start = index;
		}
	}
	if (end - start > 0) {
		strncpy(output, input + start + 1, end - start - 1);
		return output;
	}
	else
		return "\0";
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
	sendPara.myUsername = username;
	sendPara.prime1 = prime1;
	sendPara.prime2 = prime2;
	sendPara.myPublicKey = publicKey; //TEMP: Should be replaced with the list.
	hThreadSend = CreateThread(NULL, 0, SendThread, &sendPara, 0, &sendThreadId);

	// Create thread for recieving. 
	// Assign parameters. 
	receiveThreadPara receivePara;
	receivePara.clientSock = connect_sock;
	receivePara.prime1 = prime1;
	receivePara.prime2 = prime2;
	receivePara.privateKey = privateKey;
	hThreadReceive = CreateThread(NULL, 0, ReceiveThread, &receivePara, 0, &receiveThreadId);

	WaitForSingleObject(hThreadSend, INFINITE);
	WaitForSingleObject(hThreadReceive, INFINITE);

	closesocket(connect_sock);
	WSACleanup();
	system("pause");
	return 0;
}