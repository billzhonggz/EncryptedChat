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
//vector<SOCKET> clients;

typedef struct ClientList
{
	SOCKET sock;
	sockaddr_in addr;
	ofstream* out;
	char *sender;
	char *receiver;
	int clientIndex;
	char *publickey;
	struct ClientList *next;	
} ClientList;


char *extract_Sender(char *input, char *output);
char *extract_Receiver(char *input, char *output);
char *extract_Message(char *input, char *output);
char *extract_Publickey(char *input, char *output);
char *return_Publickey(ClientList *pHead);
void initList(ClientList **pNode);
ClientList *creatList(ClientList *pHead);
void printList(ClientList *pHead);
void clearList(ClientList *pHead);
int sizeList(ClientList *pHead);
SOCKET getSocket(ClientList *pHead, int pos);
char *getSender(ClientList *pHead, int pos);
char *getReceiver(ClientList *pHead, int pos);
int getIndex(ClientList *pHead, int pos);
int getSendIndex(ClientList *pHead, char *sender);
int getRecvIndex(ClientList *pHead, char *receiver);
int modifySender(ClientList *pNode, int pos, char *x);
int modifyReceiver(ClientList *pNode, int pos, char *x);
int modifyPublickey(ClientList *pNode, int pos, char *x);
int isEmptyList(ClientList *pHead);
int insertHeadList(ClientList **pNode, SOCKET sock, sockaddr_in addr, ofstream* out, char *sender, char *receiver, int clientIndex);
int insertLastList(ClientList **pNode, SOCKET sock, sockaddr_in addr, ofstream* out, char *sender, char *receiver, int clientIndex);



DWORD WINAPI clientThread(LPVOID lpParam)
{
	ClientList* clientList = (ClientList*)lpParam;
	//SOCKET sock = clientList->sock;
	int listIndex = 0;
	listIndex = getIndex(clientList, sizeList(clientList));
	SOCKET sock = getSocket(clientList, listIndex);


	char buf[DEFAULT_BUFFER] = "";
	char sendback[DEFAULT_BUFFER] = "";
	char message[DEFAULT_BUFFER] = "";
	int ret, left, idx = 0;
	char sender[17] = { '\0' };
	char receiver[17] = { '\0' };
	char publickey[DEFAULT_BUFFER] = { '\0' };

	/*int tempIndex = 0;
	tempIndex = clientList->clientIndex;

	printf("obj->clientIndex:%d\n", clientList->clientIndex); 基本不要，等下删*/
	
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

		//提取关键字
		extract_Sender(buf, sender);
		
		// TODO: 响应hello message的请求

		modifySender(clientList, listIndex, sender);

		if (extract_Receiver(buf, receiver) != "\0")
		{
			modifyReceiver(clientList, listIndex, receiver);
		}

		if (extract_Publickey(buf, publickey) != "\0")
		{
			modifyPublickey(clientList, listIndex, publickey);
		}

		strcpy(sendback, buf);

		printf("[The message from %s]:%s\n", sender, buf);

		printf("[The sendback message is]:  %s\n", sendback);

		extract_Message(buf, message);

		//printf("username:%s\n", username);
		/*clientList->username = username;
		printf("obj->username:%s\n", clientList->username);
		printf("obj->socket:%d\n", clientList->sock);
		//printf("obj->clientIndex:%d\n", obj->clientIndex);这段好可疑，等下修改*/

		if (strcmp(receiver, "server") == 0) {
			if (strcmp(message, "list") == 0) {
				printList(clientList);
				memset(sendback, 0, DEFAULT_BUFFER);
				strcat(sendback, "[server]");
				strcat(sendback, return_Publickey(clientList));
				printf("%s", sendback);
			}
			// Private message.
			for (int i = 1; i <= sizeList(clientList); i++)
			{

				left = strlen(sendback);
				while (left > 0)
				{
					//Warning: This instruction may result in crash
					ret = send(getSocket(clientList, getSendIndex(clientList, sender)), &sendback[idx], left, 0);

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
		}

		else if (getReceiver(clientList, listIndex) == NULL || strcmp(receiver, "broadcast") == 0) {

			// Boardcast message.
			for (int i = 1; i <= sizeList(clientList); i++)
			{
				left = strlen(sendback);
				while (left > 0)
				{
					ret = send(getSocket(clientList, i), &sendback[idx], left, 0);

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
		}
		else { 
			// Private message.
			for (int i = 1; i <= sizeList(clientList); i++)
			{

				left = strlen(sendback);
				while (left > 0)
				{
					ret = send(getSocket(clientList, getRecvIndex(clientList, receiver)), &sendback[idx], left, 0);

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
		}
		memset(buf, 0, DEFAULT_BUFFER);
		memset(sendback, 0, DEFAULT_BUFFER);
		memset(receiver, 0, 17);
		memset(message, 0, DEFAULT_BUFFER);
	}

	return 0;
}



int main(void) {

	//char szBuff[100];
	//int msg_len;
	int addr_len;
	struct sockaddr_in local, client_addr;

	SOCKET listensock, client;
	WSADATA wsaData;
	HANDLE hThread;
	DWORD  threadId;

	ClientList *startNode = NULL; 
	initList(&startNode);//Initialize an empty linklist for Clientlist
	//insertHeadList(&startNode, NULL, NULL, NULL);

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
		//clients.push_back(client);


		if (client == INVALID_SOCKET)
		{
			fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
			break;
		}

		if (sizeList(startNode) == 0)
		{
			insertHeadList(&startNode, client, client_addr, NULL, NULL,NULL, 1);			
		}
		else
		{
			insertLastList(&startNode, client, client_addr, NULL, NULL, NULL, sizeList(startNode) + 1);  //表尾插入元素
		}
		//insertLastList(&startNode, client, client_addr, NULL, NULL, sizeList(startNode));  //表尾插入元素
		//printf("Online client number: %d\n", clients.size());
		printf("Online client number: %d\n", sizeList(startNode)); //减去开头空节点
		
		/*
		ClientList clientList;
		clientList.addr = client_addr;
		clientList.sock = client; 原代码*/
		//为每个客户建立处理线程
		hThread = CreateThread(NULL, 0, clientThread, (LPVOID)startNode, 0, &threadId);

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

/*----------------------------------------------Main 的分割线-----------------------------------------------------------------*/

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

char *extract_Publickey(char *input, char *output)
{	
	int index = 0;
	int start = 0, end = strlen(input);
	for (index; index < end; index++)
	{
		if (input[index] == ')') {
			start = index;
		}
	}
	if (end - start > 0 && start != 0) {
		strncpy(output, input + start + 1, end - start - 1);
		return output;
	}
	else
		return "\0";
}

char *return_Publickey(ClientList *pHead) {
	char temp[DEFAULT_BUFFER] = "";
	if (NULL == pHead)   //链表为空
	{
		printf("return_Publickey函数执行，链表为空\n");
	}
	else
	{
		strcat(temp, "\n");
		while (NULL != pHead)
		{

			strcat(temp, pHead->sender);
			strcat(temp, "\t");
			strcat(temp, pHead->publickey);
			strcat(temp, "\n");
			//printf("┃     %d         %s             %s     ┃\n", pHead->clientIndex, pHead->sender, pHead->publickey);
			pHead = pHead->next;
		}
	}
	return temp;
}

/* 1.初始化线性表，即置单链表的表头指针为空 */
void initList(ClientList **pNode)
{
	*pNode = NULL;
	printf("initList函数执行，初始化成功\n");
}

/* 2.创建线性表，此函数输入负数终止读取数据*/
ClientList *creatList(ClientList *pHead)
{
	ClientList *head;

	head = (ClientList *)malloc(sizeof(ClientList)); //申请新节点
	if (head == NULL)
	{
		printf("内存分配失败\n");
		exit(0);
	}
	memset(head, 0, sizeof(ClientList));

	head->next = NULL;         //新节点的指针置为空

	printf("creatList函数执行，链表创建成功\n");
	return pHead;           //返回链表的头指针
}

/* 3.打印链表，链表的遍历*/
void printList(ClientList *pHead)
{
	if (NULL == pHead)   //链表为空
	{
		printf("PrintList函数执行，链表为空\n");
	}
	else
	{
		printf("┏----------------------------------------┓\n");
		printf("┃    No.       Client Name     Public Key┃\n");
		
		while (NULL != pHead)
		{
			printf("┃     %d         %s             %s     ┃\n", pHead->clientIndex, pHead->sender, pHead->publickey);
			pHead = pHead->next;
		}
		printf("┗----------------------------------------┛\n");
		printf("\n");
	}
}

/* 4.清除线性表L中的所有元素，即释放单链表L中所有的结点，使之成为一个空表 */
void clearList(ClientList *pHead)
{
	ClientList *pNext;            //定义一个与pHead相邻节点

	if (pHead == NULL)
	{
		printf("clearList函数执行，链表为空\n");
		return;
	}
	while (pHead->next != NULL)
	{
		pNext = pHead->next;//保存下一结点的指针
		free(pHead);
		pHead = pNext;      //表头下移
	}
	printf("clearList函数执行，链表已经清除\n");
}

/* 5.返回单链表的长度 */
int sizeList(ClientList *pHead)
{
	int size = 0;

	while (pHead != NULL)
	{
		size++;         //遍历链表size大小比链表的实际长度小1
		pHead = pHead->next;
	}
	//printf("sizeList函数执行，链表长度 %d \n", size);
	return size;    //链表的实际长度
}

/* 6.检查单链表是否为空，若为空则返回１，否则返回０ */
int isEmptyList(ClientList *pHead)
{
	if (pHead == NULL)
	{
		printf("isEmptyList函数执行，链表为空\n");
		return 1;
	}
	printf("isEmptyList函数执行，链表非空\n");

	return 0;
}

/* 7.返回单链表中第pos个结点中的元素，若pos超出范围，则停止程序运行 */
SOCKET getSocket(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getSocket函数执行，pos值非法\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getSocket函数执行，链表为空\n");
		return 0;
		//exit(1);
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("getSocket函数执行，pos值超出链表长度\n");
		return 0;
	}

	return pHead->sock;
}

/* 7.返回单链表中第pos个结点中的元素，若pos超出范围，则停止程序运行 */
int getIndex(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getIndex函数执行，pos值非法\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getIndex函数执行，链表为空\n");
		return 0;
		//exit(1);
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("getIndex函数执行，pos值超出链表长度\n");
		return 0;
	}

	return pHead->clientIndex;
}

/* 7.返回单链表中第pos个结点中的元素，若pos超出范围，则停止程序运行 */
char *getSender(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getSender函数执行，pos值非法\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getSender函数执行，链表为空\n");
		return 0;
		//exit(1);
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("getSender函数执行，pos值超出链表长度\n");
		return 0;
	}

	return pHead->sender;
}

/* 7.返回单链表中第pos个结点中的元素，若pos超出范围，则停止程序运行 */
char *getReceiver(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getReceiver函数执行，pos值非法\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getReceiver函数执行，链表为空\n");
		return 0;
		//exit(1);
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("getReceiver函数执行，pos值超出链表长度\n");
		return 0;
	}

	return pHead->receiver;
}

/* 8.从单链表中查找具有给定值x的第一个元素，若查找成功则返回该结点data域的存储地址，否则返回NULL */
int getSendIndex(ClientList *pHead, char *sender)
{
	if (NULL == pHead)
	{
		printf("getSendIndex函数执行，链表为空\n");
		return NULL;
	}
	if (sender == '\0')
	{
		printf("getSendIndex函数执行，给定值X不合法\n");
		return NULL;
	}
	while (strcmp(pHead->sender, sender) != 0 && (NULL != pHead->next)) //判断是否到链表末尾，以及是否存在所要找的元素
	{
		pHead = pHead->next;
	}
	if (strcmp(pHead->sender, sender) != 0 && (pHead != NULL))
	{
		printf("getSendIndex函数执行，在链表中未找到x值\n");
		return NULL;
	}
	if (strcmp(pHead->sender, sender) == 0)
	{
		//printf("getRecvIndex函数执行，元素 %s 的index为 %d\n", receiver, pHead->clientIndex);
	}

	return pHead->clientIndex; //返回元素的地址
}

/* 8.从单链表中查找具有给定值x的第一个元素，若查找成功则返回该结点data域的存储地址，否则返回NULL */
int getRecvIndex(ClientList *pHead, char *receiver)
{
	if (NULL == pHead)
	{
		printf("getRecvIndex函数执行，链表为空\n");
		return NULL;
	}
	if (receiver == '\0')
	{
		printf("getRecvIndex函数执行，给定值X不合法\n");
		return NULL;
	}
	while (strcmp(pHead->sender, receiver) != 0 && (NULL != pHead->next)) //判断是否到链表末尾，以及是否存在所要找的元素
	{
		pHead = pHead->next;
	}
	if (strcmp(pHead->sender, receiver)!=0 && (pHead != NULL))
	{
		printf("getRecvIndex函数执行，在链表中未找到x值\n");
		return NULL;
	}
	if (strcmp(pHead->sender, receiver) == 0)
	{
		//printf("getRecvIndex函数执行，元素 %s 的index为 %d\n", receiver, pHead->clientIndex);
	}

	return pHead->clientIndex; //返回元素的地址
}

/* 9.把单链表中第pos个结点的值修改为x的值，若修改成功返回１，否则返回０ */
int modifySender(ClientList *pNode, int pos, char *x)
{
	ClientList *pHead;
	pHead = pNode;
	int i = 0;

	if (NULL == pHead)
	{
		printf("modifySender函数执行，链表为空\n");
	}
	if (pos < 1)
	{
		printf("modifySender函数执行，pos值非法\n");
		return 0;
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("modifySender函数执行，pos值超出链表长度\n");
		return 0;
	}
	pNode = pHead;
	pNode->sender = x;
	return 1;
}

/* 9.把单链表中第pos个结点的值修改为x的值，若修改成功返回１，否则返回０ */
int modifyReceiver(ClientList *pNode, int pos, char *x)
{
	ClientList *pHead;
	pHead = pNode;
	int i = 0;

	if (NULL == pHead)
	{
		printf("modifyElem函数执行，链表为空\n");
	}
	if (pos < 1)
	{
		printf("modifyElem函数执行，pos值非法\n");
		return 0;
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("modifyReceiver函数执行，pos值超出链表长度\n");
		return 0;
	}
	pNode = pHead;
	pNode->receiver = x;

	return 1;
}

int modifyPublickey(ClientList *pNode, int pos, char *x)
{
	ClientList *pHead;
	pHead = pNode;
	int i = 0;

	if (NULL == pHead)
	{
		printf("modifyPublickey函数执行，链表为空\n");
	}
	if (pos < 1)
	{
		printf("modifyPublickey函数执行，pos值非法\n");
		return 0;
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //移到下一结点
	}
	if (i < pos)                  //链表长度不足则退出
	{
		printf("modifyReceiver函数执行，pos值超出链表长度\n");
		return 0;
	}
	pNode = pHead;
	pNode->publickey = x;

	return 1;
}

/* 10.向单链表的表头插入一个元素 */
int insertHeadList(ClientList **pNode, SOCKET sock, sockaddr_in addr, ofstream* out, char *sender, char *receiver, int clientIndex)
{
	ClientList *pInsert;
	pInsert = (ClientList *)malloc(sizeof(ClientList));
	memset(pInsert, 0, sizeof(ClientList));
	pInsert->sock = sock;
	pInsert->sender = sender;
	pInsert->receiver = receiver;
	pInsert->clientIndex = clientIndex;
	pInsert->next = *pNode;
	*pNode = pInsert;

	return 1;
}

/* 11.向单链表的末尾添加一个元素 */
int insertLastList(ClientList **pNode, SOCKET sock, sockaddr_in addr, ofstream* out, char *sender, char *receiver, int clientIndex)
{
	ClientList *pInsert;
	ClientList *pHead;
	ClientList *pTmp; //定义一个临时链表用来存放第一个节点

	pHead = *pNode;
	pTmp = pHead;
	pInsert = (ClientList *)malloc(sizeof(ClientList)); //申请一个新节点
	memset(pInsert, 0, sizeof(ClientList));
	pInsert->sock = sock;
	pInsert->addr = addr;
	pInsert->out = out;
	pInsert->sender = sender;
	pInsert->receiver = receiver;
	pInsert->clientIndex = clientIndex;

	while (pHead->next != NULL)
	{
		pHead = pHead->next;
	}
	pHead->next = pInsert;   //将链表末尾节点的下一结点指向新添加的节点
	*pNode = pTmp;

	return 1;
}