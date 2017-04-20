#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"rsa.h"

long int e[100], d[100];

// Generate RSA key pair with inputted prime numbers.
// Return long int array, first value is e, second value is d. 
long int* rsaGenKeyPair(long int prime1, long int prime2)
{
	long int r = (prime1 - 1) * (prime2 - 1);
	calcE(r, prime1, prime2);
	long int keyPair[2] = { e[0],d[0] };
	return keyPair;
}

// Check input is a prime number. 
int prime(long int pr) {
	int i;
	long int j = sqrt(pr);
	for (i = 2; i <= j; i++) {
		if (pr % i == 0)
			return 0;
	}
	return 1;
}

// Calculate value of e. 
// Variables n and r are defined by RSA.
// Value of n and r are calculated above.
void calcE(long int r, long int prime1, long int prime2)
{
	int k = 0;
	int isPrime = 0;
	long int possibleD = 0;
	for (int i = 2; i < r; i++)
	{
		if (r % i == 0)
			continue;
		isPrime = prime(i);
		if (isPrime == 1 && i != prime1 && i != prime2)
		{
			// Write all possible values of e into the array.
			e[k] = i;
			possibleD = calcD(e[k], r);
			if (possibleD > 0)
			{
				// Assign possible d value to the array.
				d[k] = possibleD;
				k++;
			}
			if (k == 99)
				break;
		}
	}
}

// Calculate value of d.
long int calcD(long int x, long int r)
{
	long int k = 1;
	while (1)
	{
		k = k + r;
		if (k % x == 0)
			return (k / x);
	}
}

// Do RSA encryption with the public key (e).
char* doEncrypt(char* msg, long int prime1, long int prime2, long int key)
{
	int len = strlen(msg);
	int i = 0;
	long int plaintext;
	long int ciphertext;
	long int k;
	long int temp[DEFAULT_BUFFER];
	long int n = prime1 * prime2;
	char *encryptedText = (char*)malloc(DEFAULT_BUFFER);
	while (i != len)
	{
		plaintext = msg[i];
		plaintext = plaintext - 96;
		k = 1;
		for (int j = 0; j < key; j++)
		{
			k = k * plaintext;
			k = k % n;
		}
		temp[i] = k;
		ciphertext = k + 96;
		encryptedText[i] = ciphertext;
		i++;
	}
	encryptedText[i] = '\0';
	return encryptedText;
}

// Do RSA decryption with private key d.
// TODO: Modify encryption algorithm to get rid of global variable: temp[]. 
char* doDecrypt(const char *encryptedMsg, long int prime1, long int prime2, long int key)
{
	long int plainText;
	long int cipherText;
	long int k;
	int i = 0;
	long int n = prime1 * prime2;
	long int temp[DEFAULT_BUFFER];
	char *decryptedMsg = (char*)malloc(DEFAULT_BUFFER);
	//int len = strlen(encryptedMsg);
	while (encryptedMsg[i] != '\0')
	{
		cipherText = encryptedMsg[i];
		k = 1;
		for (int j = 0; j < key; j++)
		{
			k = k * cipherText;
			k = k % n;
		}
		plainText = k + 96;
		//plainText = lround(pow(cipherText, key)) % n;
		decryptedMsg[i] = plainText;
		i++;
	}
	decryptedMsg[i] = '\0';
	return decryptedMsg;
}