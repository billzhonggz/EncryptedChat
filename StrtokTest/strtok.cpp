#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int* extract_Keys(char *input)
{
	long int ret[3] = { 0,0,0 };
	char publicKey[sizeof(long)] = "";
	char prime1[sizeof(int)] = "";
	char prime2[sizeof(int)] = "";
	sscanf(input, "%s,%s,%s", publicKey, prime1, prime2);
	ret[0] = atoi(publicKey);
	ret[1] = atoi(prime1);
	ret[2] = atoi(prime2);
	return ret;
}

char *splitRows(char *input)
{
	return strchr(input, '\n') + 1;
}

int main(void)
{
	char *str = "\nUser1\t11,5,7\nUser2\t11,7,5\n";
	str = strchr(str, '\n') + 1;
	char *listStr = "";
	char *username = "";
	char *keysStr = "";
	int strIndex = 0;
	int listStrIndex = 0;
	int len = strlen(str);
	while (strIndex < len)
	{
		// Split row
		if (str[strIndex] == '\n')
		{
			strncpy(listStr, str, strIndex - 1);
			printf(listStr);
			// Split username and keys.
			while (listStrIndex < strlen(listStr))
			{
				if (listStr[listStrIndex] == '\t')
				{
					strncpy(username, listStr, listStrIndex - 1);
					printf(username);
					strncpy(keysStr, listStr + listStrIndex, strlen(listStr) - 1);
					printf(keysStr);
					break;
				}
				listStrIndex++;
			}
		}
		strIndex++;
	}
	system("pause");
	return 0;
}