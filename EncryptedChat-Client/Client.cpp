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

typedef struct userNode
{
	char *username;
	long int publicKey;
	int prime1;
	int prime2;
	struct userNode *next;
}userNode;

userNode *startNode = NULL;

int firstStratFlag = 1;
char *extract_Sender(char *input, char *output);
char *extract_Receiver(char *input, char *output);
char *extract_Message(char *input, char *output, char endSign);
long int* extract_Keys(char *input);

void initUserList(userNode **pNode);
userNode *addUserToList(userNode *pNode, char *username, long int publicKey, int prime1, int prime2);
long int* findKeysByUsername(userNode *pHead, char *username);
void printList(userNode *pHead);
void clearList(userNode *pHead);

// Initialize user list.
void initUserList(userNode **pNode)
{
	*pNode = NULL;
	printf("User list initialized.\n");
}

// Add a new user.
userNode *addUserToList(userNode *pNode, char *username, long int publicKey, int prime1, int prime2)
{
	userNode *p1, *p2;
	p1 = (userNode*)malloc(sizeof(userNode));
	p1->username = (char*)malloc(4096 * sizeof(char));

	// Insert elements.
	strcpy(p1->username, username);
	p1->publicKey = publicKey;
	p1->prime1 = prime1;
	p1->prime2 = prime2;

	p2 = pNode;
	// Empty list
	if (pNode == NULL)
	{
		pNode = p1;
		p1->next = NULL;
	}
	// Non-empty list, add at the end.
	else
	{
		while (p2->next != NULL)
		{
			p2 = p2->next;
		}
		// Add node at the end.
		p2->next = p1;
		p1->next = NULL;
	}
	return pNode;
}

// Find a user by username, return keys.
long int* findKeysByUsername(userNode *pHead, char *username)
{
	long int ret[3] = { 0,0,0 };
	if (pHead == NULL)
	{
		printf("In findKeysByUsername function. List is empty.\n");
		return NULL;
	}
	// Look up the whole list.
	while ((strcmp(pHead->username, username) != 0) && (pHead->next != NULL))
	{
		pHead = pHead->next;
	}
	if ((strcmp(pHead->username, username) != 0) && (pHead != NULL))
	{
		printf("In findKeysByUsername function. Cannot find the user.\n");
		return NULL;
	}
	if (strcmp(pHead->username, username) == 0)
	{
		// Found and return.
		ret[0] = pHead->publicKey;
		ret[1] = pHead->prime1;
		ret[2] = pHead->prime2;
	}
	return ret;
}

// Print list to the screen.
void printList(userNode *pHead)
{
	if (pHead == NULL)
		printf("In printList, User list is empty!\n");
	else
	{
		printf("Current user list is:\nUsername\tpublickey\tprime1\tprime2\n");
		while (pHead != NULL)
		{
			printf("%s\t%d\t%d\t%d\n", pHead->username, pHead->publicKey, pHead->prime1, pHead->prime2);
			pHead = pHead->next;
		}
	}
}

// Clear user list.
void clearList(userNode *pHead)
{
	userNode *pNext;

	if (pHead == NULL)
	{
		printf("In clearList, list is empty.\n");
		return;
	}
	else
	{
		while (pHead->next != NULL)
		{
			pNext = pHead->next;
			free(pHead);
			pHead = pNext;
		}
		printf("In cleatList, the list is cleared.\n");
	}
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
	char zhongjunru[DEFAULT_BUFFER] = "";
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
			printf("Sending keys to the server.\n");
			strcpy(dest, "server");
			itoa(publicKey,publicKeyStr, 10);
			itoa(prime1, prime1Str, 10);
			itoa(prime2, prime2Str, 10);
			sprintf(input, "(publickey)%s,%s,%s", publicKeyStr, prime1Str, prime2Str);
			firstStratFlag = 2;
			Sleep(100);
		}
		else if (firstStratFlag == 2)
		{
			printf("Gaining online users.\n");
			strcpy(dest, "server");
			strcpy(input, "list");
			firstStratFlag = 0;
			Sleep(100);
		}
		else
		{
			// Ask for destination.
			printf("Input a receiver. \nInput \"server\" to execute server commands.\nInput \"broadcast\" to broadcast to all users.\n");
			printf("Receiver: ");
			scanf("%s", &dest);
			getchar();
			printf("Receiver is %s\n", dest);
			// Ask for input.
			printf("Input your message or server command.\n");
			printf("Message or command: ");
			scanf("%s", &input);
			getchar();
			printf("Message: ", input);
			Sleep(100);
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

		if (strcmp(dest, "server") != 0 && strcmp(dest, "broadcast") != 0)
		{
			// Find a user.
			long int *keys = NULL;
			keys = findKeysByUsername(startNode, dest);
			if (*keys == NULL)
			{
				printf("Cannot find a user, please try again.\n");
				return 1;
			}
			// Do encryption.
			char *encryptedInput = doEncrypt(input, keys[1], keys[2], keys[0]);
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
			char *originalMsg = (char*)calloc(DEFAULT_BUFFER,sizeof(char));
			//divideUsernameMessage(recvbuf, sourceUsername, encryptedMsg);
			extract_Sender(recvbuf, sourceUsername);
			extract_Receiver(recvbuf, destUsername);
			if (strcmp(destUsername, "") == 0)
				extract_Message(recvbuf, originalMsg, ']');
			else
				extract_Message(recvbuf, originalMsg, '}');
			printf("\nSource username %s, destnation %s, original message: %s\n", sourceUsername, destUsername, originalMsg);

			if (strcmp(sourceUsername,"server") != 0 && strcmp(destUsername, "broadcast") != 0)
			{
				// Do decryption.
				char *decryptedMsg = doDecrypt(originalMsg, prime1, prime2, privateKey);
				printf("%s said: %s\n", sourceUsername, decryptedMsg);
				free(decryptedMsg);
			}
			else if (strcmp(sourceUsername, "server") == 0 && strcmp(destUsername, "broadcast") != 0)
			{
				printf(originalMsg);
				// Do refresh list.
				printf("Refresh user list.\n");
				clearList(startNode);
				initUserList(&startNode);
				char username[DEFAULT_BUFFER] = "";
				char keysStr[DEFAULT_BUFFER] = "";
				long int *keys;
				originalMsg = strchr(originalMsg, '\n');
				while (strchr(originalMsg,'\n'))
				{
					// Identify one pair.
					sscanf(originalMsg, "%s\t%\s\n", &username, &keysStr);
					// Move to the next pair.
					originalMsg = strchr(originalMsg, '\n') + 1;
					keys = extract_Keys(keysStr);
					long publickey = keys[0];
					int prime1 = keys[1];
					int prime2 = keys[2];
					startNode = addUserToList(startNode, username, publickey, prime1, prime2);
				}
				printList(startNode);
			}
			else
			{
				printf("%s broadcasted: %s\n", sourceUsername, originalMsg);
			}
		}
		// Reset receive containers. 
		bytesRecv = SOCKET_ERROR;
		memset(recvbuf, 0, DEFAULT_BUFFER);
		printf("Receiver: ");
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

char *extract_Message(char *input, char *output, char endSign)
{
	int index = 0;
	int start = 0, end = strlen(input);
	for (index; index < end; index++) 
	{
		if (input[index] == endSign) {
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

void extra_KeyList(char *input)
{
	char listStr[DEFAULT_BUFFER] = "";
	// Find the first "\n" sign and split it.
	int index = 0;
	int start = 0, end = strlen(input);
	for (index; index < end; index++)
	{
		if (input[index] == '\n') {
			start = index;
			break;
		}
	}
	if (end - start > 0) {
		strncpy(listStr, input + start + 1, end - start - 1);
	}

}

// Input a string in format "publickey,prime1,prime2" and extract in an array.
// Three elements of array are publickey, prime1 and prime2. 
long int* extract_Keys(char *input)
{
	long int ret[3] = { 0,0,0 };
	char publicKey[sizeof(long)] = "";
	char prime1[sizeof(int)] = "";
	char prime2[sizeof(int)] = "";
	sscanf(input, "%[0-9],%[0-9],%[0-9]", &publicKey, &prime1, &prime2);
	ret[0] = atoi(publicKey);
	ret[1] = atoi(prime1);
	ret[2] = atoi(prime2);
	return ret;
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
	
	// Initialize list.
	initUserList(&startNode);

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