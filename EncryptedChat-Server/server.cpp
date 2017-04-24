/*
Encrypted Chatroom Server
Assignment 2
Internet & World Wide Web
1430003013 ��ع�� & 1430003045 �Ӿ���
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


//�ͻ����ӳأ���������ͻ�������
//vector<SOCKET> clients;

typedef struct ClientList
{
	SOCKET sock;
	sockaddr_in addr;
	ofstream* out;
	char *sender;
	char *receiver;
	int clientIndex;
	struct ClientList *next;
	int publickey;
} ClientList;


char *extract_Sender(char *input, char *output);
char *extract_Receiver(char *input, char *output);
void initList(ClientList **pNode);
ClientList *creatList(ClientList *pHead);
void printList(ClientList *pHead);
void clearList(ClientList *pHead);
int sizeList(ClientList *pHead);
SOCKET getSocket(ClientList *pHead, int pos);
char *getSender(ClientList *pHead, int pos);
char *getReceiver(ClientList *pHead, int pos);
int getIndex(ClientList *pHead, int pos);
int getRecvIndex(ClientList *pHead, char *receiver);
int modifySender(ClientList *pNode, int pos, char *x);
int modifyReceiver(ClientList *pNode, int pos, char *x);
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
	int ret, left, idx = 0;
	char sender[17] = { '\0' };
	char receiver[17] = { '\0' };

	/*int tempIndex = 0;
	tempIndex = clientList->clientIndex;

	printf("obj->clientIndex:%d\n", clientList->clientIndex); ������Ҫ������ɾ*/
	
	//ѭ������ͻ��˷��͵���Ϣ
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

		//��ȡ�ؼ���
		extract_Sender(buf, sender);
		
		// TODO: ��Ӧhello message������

		modifySender(clientList, listIndex, sender);

		if (extract_Receiver(buf, receiver) != "\0")
		{
			modifyReceiver(clientList, listIndex, receiver);
		}		

		sprintf(sendback, "[%s said]:%s", sender, buf);

		printf("[The message from %s]:%s\n", sender, buf);

		printf("[The sendback message is]:  %s\n", sendback);

		//printf("username:%s\n", username);
		/*clientList->username = username;
		printf("obj->username:%s\n", clientList->username);
		printf("obj->socket:%d\n", clientList->sock);
		//printf("obj->clientIndex:%d\n", obj->clientIndex);��κÿ��ɣ������޸�*/

		if (getReceiver(clientList, listIndex) == NULL) {

			// Boardcast message.
			for (int i = 1; i <= sizeList(clientList); i++)
			{
				if (strcmp(receiver, "server") == 0)
				{
					printList(clientList);
				}
				else
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
			insertLastList(&startNode, client, client_addr, NULL, NULL, NULL, sizeList(startNode) + 1);  //��β����Ԫ��
		}
		//insertLastList(&startNode, client, client_addr, NULL, NULL, sizeList(startNode));  //��β����Ԫ��
		//printf("Online client number: %d\n", clients.size());
		printf("Online client number: %d\n", sizeList(startNode)); //��ȥ��ͷ�սڵ�
		
		/*
		ClientList clientList;
		clientList.addr = client_addr;
		clientList.sock = client; ԭ����*/
		//Ϊÿ���ͻ����������߳�
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

/*----------------------------------------------Main �ķָ���-----------------------------------------------------------------*/

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

/* 1.��ʼ�����Ա����õ�����ı�ͷָ��Ϊ�� */
void initList(ClientList **pNode)
{
	*pNode = NULL;
	printf("initList����ִ�У���ʼ���ɹ�\n");
}

/* 2.�������Ա��˺������븺����ֹ��ȡ����*/
ClientList *creatList(ClientList *pHead)
{
	ClientList *head;

	head = (ClientList *)malloc(sizeof(ClientList)); //�����½ڵ�
	if (head == NULL)
	{
		printf("�ڴ����ʧ��\n");
		exit(0);
	}
	memset(head, 0, sizeof(ClientList));

	head->next = NULL;         //�½ڵ��ָ����Ϊ��

	printf("creatList����ִ�У��������ɹ�\n");
	return pHead;           //���������ͷָ��
}

/* 3.��ӡ��������ı���*/
void printList(ClientList *pHead)
{
	if (NULL == pHead)   //����Ϊ��
	{
		printf("PrintList����ִ�У�����Ϊ��\n");
	}
	else
	{
		while (NULL != pHead)
		{
			printf("Print Receiver: %s\tPrint Receiver: %s\tPrint index: %d\n", pHead->sender, pHead->receiver, pHead->clientIndex);
			pHead = pHead->next;
		}
		printf("\n");
	}
}

/* 4.������Ա�L�е�����Ԫ�أ����ͷŵ�����L�����еĽ�㣬ʹ֮��Ϊһ���ձ� */
void clearList(ClientList *pHead)
{
	ClientList *pNext;            //����һ����pHead���ڽڵ�

	if (pHead == NULL)
	{
		printf("clearList����ִ�У�����Ϊ��\n");
		return;
	}
	while (pHead->next != NULL)
	{
		pNext = pHead->next;//������һ����ָ��
		free(pHead);
		pHead = pNext;      //��ͷ����
	}
	printf("clearList����ִ�У������Ѿ����\n");
}

/* 5.���ص�����ĳ��� */
int sizeList(ClientList *pHead)
{
	int size = 0;

	while (pHead != NULL)
	{
		size++;         //��������size��С�������ʵ�ʳ���С1
		pHead = pHead->next;
	}
	//printf("sizeList����ִ�У������� %d \n", size);
	return size;    //�����ʵ�ʳ���
}

/* 6.��鵥�����Ƿ�Ϊ�գ���Ϊ���򷵻أ������򷵻أ� */
int isEmptyList(ClientList *pHead)
{
	if (pHead == NULL)
	{
		printf("isEmptyList����ִ�У�����Ϊ��\n");
		return 1;
	}
	printf("isEmptyList����ִ�У�����ǿ�\n");

	return 0;
}

/* 7.���ص������е�pos������е�Ԫ�أ���pos������Χ����ֹͣ�������� */
SOCKET getSocket(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getElement����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getElement����ִ�У�����Ϊ��\n");
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
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("getElement����ִ�У�posֵ����������\n");
		return 0;
	}

	return pHead->sock;
}

/* 7.���ص������е�pos������е�Ԫ�أ���pos������Χ����ֹͣ�������� */
int getIndex(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getElement����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getElement����ִ�У�����Ϊ��\n");
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
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("getElement����ִ�У�posֵ����������\n");
		return 0;
	}

	return pHead->clientIndex;
}

/* 7.���ص������е�pos������е�Ԫ�أ���pos������Χ����ֹͣ�������� */
char *getSender(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getElement����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getElement����ִ�У�����Ϊ��\n");
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
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("getElement����ִ�У�posֵ����������\n");
		return 0;
	}

	return pHead->sender;
}

/* 7.���ص������е�pos������е�Ԫ�أ���pos������Χ����ֹͣ�������� */
char *getReceiver(ClientList *pHead, int pos)
{
	int i = 0;

	if (pos < 1)
	{
		printf("getElement����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	if (pHead == NULL)
	{
		printf("getElement����ִ�У�����Ϊ��\n");
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
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("getElement����ִ�У�posֵ����������\n");
		return 0;
	}

	return pHead->receiver;
}

/* 8.�ӵ������в��Ҿ��и���ֵx�ĵ�һ��Ԫ�أ������ҳɹ��򷵻ظý��data��Ĵ洢��ַ�����򷵻�NULL */
int getRecvIndex(ClientList *pHead, char *receiver)
{
	if (NULL == pHead)
	{
		printf("getElemAddr����ִ�У�����Ϊ��\n");
		return NULL;
	}
	if (receiver == '\0')
	{
		printf("getElemAddr����ִ�У�����ֵX���Ϸ�\n");
		return NULL;
	}
	while (strcmp(pHead->sender, receiver) != 0 && (NULL != pHead->next)) //�ж��Ƿ�����ĩβ���Լ��Ƿ������Ҫ�ҵ�Ԫ��
	{
		pHead = pHead->next;
	}
	if (strcmp(pHead->sender, receiver)!=0 && (pHead != NULL))
	{
		printf("getElemAddr����ִ�У���������δ�ҵ�xֵ\n");
		return NULL;
	}
	if (strcmp(pHead->sender, receiver) == 0)
	{
		printf("getElemAddr����ִ�У�Ԫ�� %s ��indexΪ %d\n", receiver, pHead->clientIndex);
	}

	return pHead->clientIndex; //����Ԫ�صĵ�ַ
}

/* 9.�ѵ������е�pos������ֵ�޸�Ϊx��ֵ�����޸ĳɹ����أ������򷵻أ� */
int modifySender(ClientList *pNode, int pos, char *x)
{
	ClientList *pHead;
	pHead = pNode;
	int i = 0;

	if (NULL == pHead)
	{
		printf("modifySender����ִ�У�����Ϊ��\n");
	}
	if (pos < 1)
	{
		printf("modifySender����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("modifySender����ִ�У�posֵ����������\n");
		return 0;
	}
	pNode = pHead;
	pNode->sender = x;
	return 1;
}

/* 9.�ѵ������е�pos������ֵ�޸�Ϊx��ֵ�����޸ĳɹ����أ������򷵻أ� */
int modifyReceiver(ClientList *pNode, int pos, char *x)
{
	ClientList *pHead;
	pHead = pNode;
	int i = 0;

	if (NULL == pHead)
	{
		printf("modifyElem����ִ�У�����Ϊ��\n");
	}
	if (pos < 1)
	{
		printf("modifyElem����ִ�У�posֵ�Ƿ�\n");
		return 0;
	}
	while (pHead != NULL)
	{
		++i;
		if (i == pos)
		{
			break;
		}
		pHead = pHead->next; //�Ƶ���һ���
	}
	if (i < pos)                  //�����Ȳ������˳�
	{
		printf("modifyReceiver����ִ�У�posֵ����������\n");
		return 0;
	}
	pNode = pHead;
	pNode->receiver = x;

	return 1;
}

/* 10.������ı�ͷ����һ��Ԫ�� */
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

/* 11.�������ĩβ���һ��Ԫ�� */
int insertLastList(ClientList **pNode, SOCKET sock, sockaddr_in addr, ofstream* out, char *sender, char *receiver, int clientIndex)
{
	ClientList *pInsert;
	ClientList *pHead;
	ClientList *pTmp; //����һ����ʱ����������ŵ�һ���ڵ�

	pHead = *pNode;
	pTmp = pHead;
	pInsert = (ClientList *)malloc(sizeof(ClientList)); //����һ���½ڵ�
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
	pHead->next = pInsert;   //������ĩβ�ڵ����һ���ָ������ӵĽڵ�
	*pNode = pTmp;

	return 1;
}