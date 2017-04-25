#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct userNode
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
	while ((strcmp(pHead->username,username) != 0) && (pHead->next != NULL))
	{
		pHead = pHead->next;
	}
	if ((strcmp(pHead->username,username) != 0) && (pHead != NULL))
	{
		printf("In findKeysByUsername function. Cannot find the user.\n");
		return NULL;
	}
	if (strcmp(pHead->username,username) == 0)
	{
		// Found and return.
		ret[0] = pHead->publicKey;
		ret[1] = pHead->prime1;
		ret[2] = pHead->prime2;
	}
	return ret;
}

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
	userNode *startNode = NULL;
	initUserList(&startNode);

	char *str = "\nUser1\t11,5,7\nUser2\t11,7,5\nUser3\t1,2,3\n";
	str = strchr(str, '\n') + 1;
	//char *listStr = "";
	char username[4096] = "";
	char keysStr[4096] = "";
	int len = strlen(str);
	long *keys;
	printf("Before into while loop; str: %s, length: %d.\n", str, len);
	while (strchr(str, '\n'))
	{
		printf("In while loop.\n");
		//str = str + index;
		printf("Now str is: %s", str);
		sscanf(str, "%s\t%s\n", &username, &keysStr);
		str = strchr(str, '\n') + 1;
		printf("username is: %s, keys are: %s\n", username, keysStr);
		keys = extract_Keys(keysStr);
		long publickey = keys[0];
		int prime1 = keys[1];
		int prime2 = keys[2];
		printf("Public key is %d, prime1 is %d, prime2 is %d.\n", keys[0], keys[1], keys[2]);
		startNode = addUserToList(startNode, username, publickey, prime1, prime2);
		printList(startNode);
	}

	system("pause");
	return 0;
}