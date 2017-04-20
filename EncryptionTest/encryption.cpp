/*
	The test project for encryption algorithm.
*/

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<cstdlib>
#include<ctime>

#define DEFAULT_BUFFER 2048

int genPrimeNum(void);
int isPrime(int n);
long rsa(int baseNum, int key, long message);

int main(void)
{
	printf("Enter a plaintext for encryption.\n");
	char plaintext[DEFAULT_BUFFER] = "";
	fgets(plaintext, DEFAULT_BUFFER, stdin);
	printf("Your input is: %s", plaintext);

	// Begin RSA encryption. 
	//int prime1 = genPrimeNum();
	//int prime2 = genPrimeNum();
	//printf("Prime number 1: %d, prime number 2: %d.\n", prime1, prime2);
	int baseNum = 3 * 11;
	int publicKey = 3;
	int privateKey = 7;
	long msg = atol(plaintext);
	printf("Converted long number %d.\n", msg);
	// Encrypetd.
	long encryMsg = rsa(baseNum, publicKey, msg);
	printf("Encrypted message is %d.\n", encryMsg);
	// Decryped.
	long decryMsg = rsa(baseNum, privateKey, encryMsg);
	printf("Decryped message is %d.\n", decryMsg);
	system("pause");
}

// Generate a prime number within 0-999.
int genPrimeNum(void)
{
	// Set system time as random seed. 
	// That ensure the random number is different.
	srand((unsigned)time(0));
	int randomNum = rand();
	// Calaculate the most close prime number. 
	int i = 1;
	int j;
	int x;
	while (randomNum > 0)
	{
		x = i;
		if (isPrime(i) == 1)
		{
			randomNum--;
			i++;
		}
		else
		{
			i++;
		}
	}
	printf("Generated the %dth prime number.\n", randomNum);
	
	return randomNum;
}

// Determine whether the number is a prime number.
int isPrime(int n)
{
	int i;
	for (i = 2; i <= n / 2; i++)
	{
		if (n%i == 0)
			return 0;
	}
	return 1;
}

long rsa(int baseNum, int key, long message)
{
	if (baseNum < 1 || key < 1)
		return 0L;
	// Data after encryption / decryption.
	long rsaMessage = 0L;
	// Core encryption.
	double rsaPow = pow(message, key);
	rsaMessage = lround(rsaPow) % baseNum;
	return rsaMessage;
}